#!/usr/bin/env python3

# ==============================================================================
# @brief Generate static repository documentation.
#
# Produces a navigable HTML documentation, Markdown rendering, copied binary
# assets, and Docgen wrappers around native Doxygen HTML API pages.
#
# @copyright
# Copyright (c) 2026 FBS93.
# See the LICENSE file of this project for license details.
# This notice shall be retained in all copies or substantial portions
# of the software.
#
# @warning
# This software is provided "as is", without any express or implied warranty.
# The user assumes all responsibility for its use and any consequences.
# ==============================================================================

# ==============================================================================
# IMPORTS
# ==============================================================================

# ------------------------------------------------------------------------------
# Standard library imports
# ------------------------------------------------------------------------------
import argparse
import os
import posixpath
import shutil
import subprocess
import sys
import tempfile
import urllib.parse
import xml.etree.ElementTree as element_tree
from collections import defaultdict
from html.parser import HTMLParser
from pathlib import Path
from stat import S_ISREG

# ------------------------------------------------------------------------------
# External library imports
# ------------------------------------------------------------------------------
from jinja2 import Environment
from jinja2 import FileSystemLoader
from jinja2 import select_autoescape
from markdown_it import MarkdownIt
from markupsafe import Markup
from mdit_py_plugins.anchors import anchors_plugin

# ------------------------------------------------------------------------------
# Project-specific imports
# ------------------------------------------------------------------------------

# ==============================================================================
# CONSTANTS
# ==============================================================================

##
# @brief Locate page templates.
##
TEMPLATE_DIRECTORY = Path(__file__).with_name("templates")
##
# @brief Locate the documentation stylesheet.
##
STYLESHEET_SOURCE = Path(__file__).with_name("assets") / "docgen.css"
##
# @brief Define the generated stylesheet location.
##
STYLESHEET_DESTINATION = Path("assets") / "docgen.css"
##
# @brief Locate the documentation client script.
##
SCRIPT_SOURCE = Path(__file__).with_name("assets") / "docgen.js"
##
# @brief Define the generated client-script location.
##
SCRIPT_DESTINATION = Path("assets") / "docgen.js"
##
# @brief Locate the Mermaid client library.
##
MERMAID_SOURCE = (
  Path(__file__).with_name("assets") / "mermaid" / "mermaid-11.12.0.min.js"
)
##
# @brief Define the generated Mermaid library location.
##
MERMAID_DESTINATION = Path("assets") / "mermaid-11.12.0.min.js"
##
# @brief Locate the Mermaid license text.
##
MERMAID_LICENSE_SOURCE = (
  Path(__file__).with_name("assets") / "mermaid" / "MERMAID-LICENSE"
)
##
# @brief Define the generated Mermaid license location.
##
MERMAID_LICENSE_DESTINATION = Path("assets") / "MERMAID-LICENSE"
##
# @brief Locate the Highlight.js client library.
##
HIGHLIGHT_SOURCE = (
  Path(__file__).with_name("assets") / "highlight" / "highlight-11.11.1.min.js"
)
##
# @brief Define the generated Highlight.js library location.
##
HIGHLIGHT_DESTINATION = Path("assets") / "highlight-11.11.1.min.js"
##
# @brief Locate the Highlight.js license text.
##
HIGHLIGHT_LICENSE_SOURCE = (
  Path(__file__).with_name("assets") / "highlight" / "HIGHLIGHT-LICENSE"
)
##
# @brief Define the generated Highlight.js license location.
##
HIGHLIGHT_LICENSE_DESTINATION = Path("assets") / "HIGHLIGHT-LICENSE"
##
# @brief Identify supported Markdown filename suffixes.
##
MARKDOWN_SUFFIXES = {".md", ".markdown", ".mdown"}
##
# @brief Identify root-level Markdown README filenames.
##
ROOT_README_NAMES = {f"readme{suffix}" for suffix in MARKDOWN_SUFFIXES}
##
# @brief Identify assembly-language filename suffixes.
##
ASSEMBLY_SUFFIXES = {".s", ".asm"}
##
# @brief Identify C and C++ filename suffixes for API generation.
##
DOXYGEN_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx"}
##
# @brief Map source suffixes to highlighting languages.
##
RAW_LANGUAGE_SUFFIXES = {
  ".asm": "armasm",
  ".bash": "bash",
  ".c": "c",
  ".cc": "cpp",
  ".cmake": "cmake",
  ".cpp": "cpp",
  ".csh": "bash",
  ".cxx": "cpp",
  ".h": "c",
  ".hh": "cpp",
  ".hpp": "cpp",
  ".hxx": "cpp",
  ".inc": "c",
  ".json": "json",
  ".jsonc": "json",
  ".json5": "json",
  **{suffix: "markdown" for suffix in MARKDOWN_SUFFIXES},
  ".mk": "makefile",
  ".py": "python",
  ".pyw": "python",
  ".s": "armasm",
  ".sh": "bash",
  ".toml": "toml",
  ".yaml": "yaml",
  ".yml": "yaml",
  ".zsh": "bash",
}
##
# @brief Map special source filenames to highlighting languages.
##
RAW_LANGUAGE_FILENAMES = {
  "cmakelists.txt": "cmake",
  "dockerfile": "dockerfile",
  "makefile": "makefile",
}

# ==============================================================================
# CLASSES
# ==============================================================================


class DocgenError(Exception):
  ##
  # @brief Represent an error that prevents trustworthy documentation output.
  ##
  pass


# ==============================================================================
# FUNCTIONS
# ==============================================================================


def parse_args():
  ##
  # @brief Parse Docgen command-line options.
  # @return Parsed command-line arguments.
  ##
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument(
    "--root",
    type=Path,
    default=Path.cwd(),
    help="Repository root (default: current directory).",
  )
  parser.add_argument(
    "--output",
    type=Path,
    default=Path("build/docgen"),
    help="Output directory, relative to --root if not absolute.",
  )
  parser.add_argument(
    "--title",
    help="Project title (default: basename of the resolved --root).",
  )
  parser.add_argument(
    "--exclude",
    action="append",
    default=[],
    metavar="PATH",
    help=(
      "Explicit basename or root-relative path to exclude (may be repeated)."
    ),
  )
  parser.add_argument(
    "--include-ignored",
    action="store_true",
    help="Include files ignored by .gitignore files.",
  )
  parser.add_argument(
    "--no-api",
    action="store_true",
    help="Do not invoke Doxygen or create API pages.",
  )
  parser.add_argument(
    "--strict",
    action="store_true",
    help="Fail on Doxygen, local-link, or symlink warnings.",
  )
  parser.add_argument(
    "--test-result",
    action="append",
    default=[],
    nargs=2,
    metavar=("LABEL", "PATH"),
    help="Add a UTF-8 raw test log as an individual Test results page.",
  )
  return parser.parse_args()


def relative_path(path, root):
  ##
  # @brief Convert a path below the repository root to POSIX form.
  # @param[in] path Path to convert.
  # @param[in] root Repository root.
  # @return Root-relative POSIX path.
  ##
  return path.relative_to(root).as_posix()


def should_exclude(path, root, output, extra):
  ##
  # @brief Return the exclusion reason for a candidate path, if any.
  # @param[in] path Candidate path.
  # @param[in] root Repository root.
  # @param[in] output Generated output path.
  # @param[in] extra User-requested exclusions.
  # @return Exclusion reason or None.
  ##
  if path == output or output in path.parents:
    return "generated output"
  rel = relative_path(path, root)
  if ".git" in path.relative_to(root).parts:
    return "Git metadata"
  if rel in extra or path.name in extra:
    return "user exclusion"
  return None


class GitIgnoreMatcher:
  ##
  # @brief Apply repository .gitignore rules through one temporary Git matcher.
  ##
  def __init__(self, root):
    ##
    # @brief Initialize a private bare repository for Git's ignore matcher.
    # @param[in] root Repository tree containing the .gitignore files.
    ##
    self.root = root
    self.temporary_directory = tempfile.TemporaryDirectory(prefix="docgen-git-")
    self.git_dir = self.temporary_directory.name
    try:
      initialization = subprocess.run(
        ["git", "init", "--bare", "--quiet", self.git_dir],
        check=False,
        text=True,
        capture_output=True,
      )
    except FileNotFoundError as error:
      self.temporary_directory.cleanup()
      raise DocgenError(
        "Git is required to apply .gitignore rules but was not found"
      ) from error
    if initialization.returncode:
      self.temporary_directory.cleanup()
      raise DocgenError(
        "cannot initialize temporary Git matcher: "
        + initialization.stderr.strip()
      )

  def close(self):
    ##
    # @brief Remove the private Git matcher repository.
    ##
    self.temporary_directory.cleanup()

  def ignored_paths(self, paths):
    ##
    # @brief Return root-relative candidate paths ignored by Git.
    # @param[in] paths Root-relative candidate paths.
    # @return Candidate paths matched by .gitignore.
    ##
    if not paths:
      return set()
    process = subprocess.run(
      [
        "git",
        "-c",
        "core.excludesFile=/dev/null",
        f"--git-dir={self.git_dir}",
        f"--work-tree={self.root}",
        "check-ignore",
        "--no-index",
        "--stdin",
        "-z",
      ],
      check=False,
      input="\0".join(paths) + "\0",
      text=True,
      capture_output=True,
    )
    if process.returncode not in {0, 1}:
      raise DocgenError(
        "Git failed while applying .gitignore rules: " + process.stderr.strip()
      )
    return {path for path in process.stdout.split("\0") if path}


def symlink_reason(path, root):
  ##
  # @brief Return the safety exclusion reason for a symbolic link.
  # @param[in] path Candidate path.
  # @param[in] root Repository root.
  # @return Exclusion reason or None.
  ##
  if not path.is_symlink():
    return None
  try:
    path.resolve().relative_to(root)
  except ValueError:
    return "symlink target outside --root"
  if path.is_dir():
    return "symlinked directory is not followed"
  return None


def discover_files(root, output, extra, include_ignored):
  ##
  # @brief Select documentable files and record exclusions and warnings.
  # @param[in] root Repository root.
  # @param[in] output Generated output path.
  # @param[in] extra User-requested exclusions.
  # @param[in] include_ignored Whether to retain .gitignore-matched paths.
  # @return Selected files, exclusions, and warnings.
  ##
  files = []
  excluded = []
  warnings = []
  matcher = None
  try:
    for current, directories, filenames in os.walk(root):
      current_path = Path(current)
      if not include_ignored and matcher is None and ".gitignore" in filenames:
        matcher = GitIgnoreMatcher(root)

      directory_candidates = []
      kept_directories = []
      for directory in directories:
        candidate = current_path / directory
        reason = should_exclude(candidate, root, output, extra) or (
          symlink_reason(candidate, root)
        )
        if reason:
          rel = relative_path(candidate, root)
          excluded.append((rel, reason, True))
          if candidate.is_symlink():
            warnings.append(f"{rel}: {reason}")
        else:
          directory_candidates.append(candidate)
      ignored_directories = (
        matcher.ignored_paths(
          [relative_path(path, root) for path in directory_candidates]
        )
        if matcher is not None
        else set()
      )
      for candidate in directory_candidates:
        rel = relative_path(candidate, root)
        if rel in ignored_directories:
          excluded.append((rel, ".gitignore", True))
        else:
          kept_directories.append(candidate.name)
      directories[:] = kept_directories

      file_candidates = []
      for filename in filenames:
        candidate = current_path / filename
        reason = should_exclude(candidate, root, output, extra) or (
          symlink_reason(candidate, root)
        )
        if reason:
          rel = relative_path(candidate, root)
          excluded.append((rel, reason, False))
          if candidate.is_symlink():
            warnings.append(f"{rel}: {reason}")
        elif candidate.is_file():
          file_candidates.append(candidate)
      ignored_files = (
        matcher.ignored_paths(
          [relative_path(path, root) for path in file_candidates]
        )
        if matcher is not None
        else set()
      )
      for candidate in file_candidates:
        rel = relative_path(candidate, root)
        if rel in ignored_files:
          excluded.append((rel, ".gitignore", False))
        else:
          files.append(candidate)
  finally:
    if matcher is not None:
      matcher.close()
  return sorted(files), sorted(set(excluded)), sorted(set(warnings))


def exclusion_frontier(selected_files, excluded):
  ##
  # @brief Return the minimal reportable boundary of excluded source paths.
  # @param[in] selected_files Selected source paths.
  # @param[in] excluded Excluded path, reason, and directory flag tuples.
  # @return Excluded path and reason tuples suitable for the generation report.
  ##
  selected = {Path(path) for path in selected_files}
  excluded_files = {
    Path(path): reason
    for path, reason, is_directory in excluded
    if not is_directory
  }
  excluded_directories = {
    Path(path): reason
    for path, reason, is_directory in excluded
    if is_directory
  }
  candidates = set(excluded_directories)
  for path in excluded_files:
    candidates.update(path.parents)
  candidates.discard(Path("."))
  collapsed = {}
  for directory in sorted(
    candidates, key=lambda path: (-len(path.parts), path)
  ):
    if any(directory in path.parents for path in selected):
      continue
    if directory in excluded_directories:
      collapsed[directory] = excluded_directories[directory]
      continue
    descendant_reasons = {
      reason
      for path, reason in excluded_files.items()
      if directory in path.parents
    }
    if len(descendant_reasons) == 1:
      collapsed[directory] = descendant_reasons.pop()
  roots = {
    directory: reason
    for directory, reason in collapsed.items()
    if not any(parent in collapsed for parent in directory.parents)
  }
  frontier = [(path.as_posix(), reason) for path, reason in roots.items()]
  frontier.extend(
    (path.as_posix(), reason)
    for path, reason in excluded_files.items()
    if not any(directory in path.parents for directory in roots)
  )
  return sorted(frontier)


def read_text(path):
  ##
  # @brief Read a UTF-8 text file while rejecting binary and invalid input.
  # @param[in] path File to read.
  # @return Decoded text or None.
  ##
  data = path.read_bytes()
  if b"\x00" in data:
    return None
  try:
    return data.decode("utf-8")
  except UnicodeDecodeError:
    return None


def validate_test_results(entries):
  ##
  # @brief Validate and read test logs before output replacement.
  # @param[in] entries Label and path pairs supplied on the command line.
  # @return Validated test-result page data.
  ##
  results = []
  for label, path_string in entries:
    path = Path(path_string)
    try:
      status = path.stat()
      if path.is_symlink() or not S_ISREG(status.st_mode):
        raise DocgenError(f"test result is not a regular file: {path}")
      content = path.read_text(encoding="utf-8")
    except UnicodeDecodeError as error:
      raise DocgenError(f"test result is not valid UTF-8: {path}") from error
    except OSError as error:
      raise DocgenError(f"cannot read test result: {path}: {error}") from error
    results.append({"label": label, "content": content})
  return results


def is_text_file(path, content):
  ##
  # @brief Determine whether decoded file content represents text.
  # @param[in] path Source path.
  # @param[in] content Decoded content or None.
  # @return True when the file is text.
  ##
  del path
  return content is not None


def raw_language(path):
  ##
  # @brief Select a deterministic raw-source highlighting language.
  # @param[in] path Source path.
  # @return Highlight language or plaintext.
  ##
  filename = path.name.lower()
  return RAW_LANGUAGE_FILENAMES.get(
    filename, RAW_LANGUAGE_SUFFIXES.get(path.suffix.lower(), "plaintext")
  )


def page_path(rel):
  ##
  # @brief Return the generated page path for a repository-relative file.
  # @param[in] rel Repository-relative path.
  # @return Generated HTML page path.
  ##
  return Path("files") / (rel + ".html")


def directory_page_path(rel):
  ##
  # @brief Return the generated page path for a non-root repository directory.
  # @param[in] rel Repository-relative directory path.
  # @return Generated directory index page path.
  ##
  return Path("files") / rel / "index.html"


def asset_path(rel):
  ##
  # @brief Return the generated asset path for a repository-relative file.
  # @param[in] rel Repository-relative path.
  # @return Generated asset path.
  ##
  return Path("assets") / rel


def href_between(source, target):
  ##
  # @brief Return a POSIX relative hyperlink between generated pages.
  # @param[in] source Source page path.
  # @param[in] target Target path.
  # @return Relative hyperlink.
  ##
  return posixpath.relpath(target.as_posix(), source.parent.as_posix())


def is_navigation_ancestor(current_page, navigation_path):
  ##
  # @brief Determine whether a navigation path contains the current source.
  # @param[in] current_page Generated page path.
  # @param[in] navigation_path Repository navigation path.
  # @return True when the navigation path is an ancestor.
  ##
  page_parts = current_page.parts
  if page_parts[:1] == ("files",):
    source_parts = page_parts[1:]
  elif page_parts[:2] == ("api", "files"):
    source_parts = page_parts[2:]
  else:
    return False
  navigation_parts = Path(navigation_path).parts
  return source_parts[: len(navigation_parts)] == navigation_parts


class LinkRewriter(HTMLParser):
  ##
  # @brief Preserve HTML while rewriting local href and src attributes.
  ##
  def __init__(self, rewrite):
    ##
    # @brief Initialize the parser with an attribute rewrite callback.
    # @param[in] rewrite Callback that rewrites a URL value.
    ##
    super().__init__(convert_charrefs=False)
    self.rewrite = rewrite
    self.parts = []

  def handle_starttag(self, tag, attrs):
    ##
    # @brief Render a start tag after rewriting local resource attributes.
    # @param[in] tag HTML tag name.
    # @param[in] attrs HTML tag attributes.
    ##
    rendered = []
    for name, value in attrs:
      if name in {"href", "src"} and value is not None:
        value = self.rewrite(value)
      rendered.append(f' {name}="{Markup.escape(value or "")}"')
    self.parts.append(f"<{tag}{''.join(rendered)}>")

  def handle_startendtag(self, tag, attrs):
    ##
    # @brief Render a self-closing tag after rewriting its attributes.
    # @param[in] tag HTML tag name.
    # @param[in] attrs HTML tag attributes.
    ##
    self.handle_starttag(tag, attrs)
    self.parts[-1] = self.parts[-1][:-1] + " />"

  def handle_endtag(self, tag):
    ##
    # @brief Preserve an HTML end tag.
    # @param[in] tag HTML tag name.
    ##
    self.parts.append(f"</{tag}>")

  def handle_data(self, data):
    ##
    # @brief Preserve HTML character data.
    # @param[in] data HTML character data.
    ##
    self.parts.append(data)

  def handle_entityref(self, name):
    ##
    # @brief Preserve a named HTML entity reference.
    # @param[in] name Entity name.
    ##
    self.parts.append(f"&{name};")

  def handle_charref(self, name):
    ##
    # @brief Preserve a numeric HTML character reference.
    # @param[in] name Character reference value.
    ##
    self.parts.append(f"&#{name};")

  def handle_comment(self, data):
    ##
    # @brief Preserve an HTML comment.
    # @param[in] data Comment content.
    ##
    self.parts.append(f"<!--{data}-->")

  def handle_decl(self, decl):
    ##
    # @brief Preserve an HTML declaration.
    # @param[in] decl Declaration content.
    ##
    self.parts.append(f"<!{decl}>")


def rewrite_html_links(
  rendered, source_rel, source_page, destinations, warnings
):
  ##
  # @brief Rewrite local HTML links to mirrored generated destinations.
  # @param[in] rendered Rendered HTML.
  # @param[in] source_rel Source repository-relative path.
  # @param[in] source_page Generated source page path.
  # @param[in] destinations Mirrored destinations by source path.
  # @param[out] warnings Documentation warnings to extend.
  # @return HTML with rewritten local links.
  ##
  def rewrite(value):
    ##
    # @brief Resolve one local URL against the mirrored destination map.
    # @param[in] value URL attribute value.
    # @return Rewritten URL value.
    ##
    parsed = urllib.parse.urlsplit(value)
    if (
      parsed.scheme
      or parsed.netloc
      or value.startswith("#")
      or value.startswith("/")
    ):
      return value
    target = posixpath.normpath(
      posixpath.join(
        posixpath.dirname(source_rel), urllib.parse.unquote(parsed.path)
      )
    )
    if target.startswith("../") or target == "..":
      warnings.append(f"{source_rel}: local link escapes repository: {value}")
      return value
    destination = destinations.get(target)
    if destination is None:
      warnings.append(f"{source_rel}: local link target not mirrored: {value}")
      return value
    suffix = f"?{parsed.query}" if parsed.query else ""
    if parsed.fragment:
      suffix += f"#{parsed.fragment}"
    return href_between(source_page, destination) + suffix

  parser = LinkRewriter(rewrite)
  parser.feed(rendered)
  parser.close()
  return "".join(parser.parts)


def create_template_environment():
  ##
  # @brief Create the Jinja environment used by generated pages.
  # @return Configured template environment.
  ##
  return Environment(
    loader=FileSystemLoader(TEMPLATE_DIRECTORY),
    autoescape=select_autoescape(["html", "j2", "xml"]),
  )


def resolve_project_title(root, title=None):
  ##
  # @brief Resolve an explicit project title or the repository basename.
  # @param[in] root Resolved repository root.
  # @param[in] title Optional explicit title.
  # @return Project title.
  ##
  return title if title is not None else root.name


def api_label(path, colliding_stems):
  ##
  # @brief Return an unambiguous navigation label for an API page.
  # @param[in] path API source path.
  # @param[in] colliding_stems Source stems requiring disambiguation.
  # @return API navigation label.
  ##
  stem = path.stem
  if stem in colliding_stems:
    return f"{stem} [API: {path.name}]"
  return stem


def build_navigation(
  files, destinations, directory_destinations, api_destinations
):
  ##
  # @brief Build the repository navigation tree for generated pages.
  # @param[in] files Repository-relative files.
  # @param[in] destinations Generated source-page destinations.
  # @param[in] directory_destinations Generated directory-page destinations.
  # @param[in] api_destinations Generated API-page destinations.
  # @return Nested navigation entries.
  ##
  tree = {}
  for path in files:
    node = tree
    for part in path.parts[:-1]:
      node = node.setdefault(part, {})
    node[path.name] = {
      "destination": destinations[path.as_posix()],
      "api_destination": api_destinations.get(path.as_posix()),
      "path": path,
    }

  def render(node, prefix=Path()):
    ##
    # @brief Render one navigation subtree into ordered entry dictionaries.
    # @param[in] node Navigation subtree.
    # @param[in] prefix Repository-relative subtree path.
    # @return Ordered navigation entries.
    ##
    entries = []
    api_stems = defaultdict(int)
    for value in node.values():
      if not isinstance(value, dict) or "destination" not in value:
        continue
      if value["api_destination"] is not None:
        api_stems[value["path"].stem] += 1
    colliding_stems = {stem for stem, count in api_stems.items() if count > 1}
    directories = sorted(
      (name, value)
      for name, value in node.items()
      if isinstance(value, dict) and "destination" not in value
    )
    files = sorted(
      (name, value)
      for name, value in node.items()
      if not (isinstance(value, dict) and "destination" not in value)
    )
    for name, value in directories:
      entries.append(
        {
          "name": name,
          "path": (prefix / name).as_posix(),
          "destination": directory_destinations[(prefix / name).as_posix()],
          "children": render(value, prefix / name),
        }
      )
    for name, value in files:
      if value["api_destination"] is not None:
        entries.append(
          {
            "name": api_label(value["path"], colliding_stems),
            "destination": value["api_destination"],
            "kind": "api",
          }
        )
      entries.append({"name": name, "destination": value["destination"]})
    return entries

  return render(tree)


def directory_entries(entries):
  ##
  # @brief Yield directory navigation entries in tree order.
  # @param[in] entries Navigation entries to traverse.
  # @return Directory navigation entries.
  ##
  for entry in entries:
    if "children" in entry:
      yield entry
      yield from directory_entries(entry["children"])


def root_readme(contents):
  ##
  # @brief Return the root-level Markdown README, if present.
  # @param[in] contents Selected file contents by relative path.
  # @return README relative path or None.
  ##
  candidates = [
    rel
    for rel in contents
    if "/" not in rel and rel.lower() in ROOT_README_NAMES
  ]
  return min(candidates, key=str.lower) if candidates else None


def create_markdown_renderer():
  ##
  # @brief Create the CommonMark renderer for Markdown documents.
  # @return Configured Markdown renderer.
  ##
  renderer = (
    MarkdownIt("commonmark", {"html": True})
    .enable("strikethrough")
    .enable("table")
    .use(anchors_plugin, min_level=1, max_level=6)
  )
  renderer.add_render_rule("table_open", render_table_open)
  renderer.add_render_rule("table_close", render_table_close)
  renderer.add_render_rule("paragraph_open", render_paragraph_open)
  return renderer


def render_table_open(renderer, tokens, index, options, environment):
  ##
  # @brief Open a horizontal scroll container before a Markdown table.
  # @param[in] renderer Markdown renderer.
  # @param[in] tokens Markdown tokens.
  # @param[in] index Current token index.
  # @param[in] options Renderer options.
  # @param[in] environment Render environment.
  # @return Rendered opening HTML.
  ##
  return '<div class="markdown-table-wrapper">\n' + renderer.renderToken(
    tokens, index, options, environment
  )


def render_table_close(renderer, tokens, index, options, environment):
  ##
  # @brief Close the scroll container after a Markdown table.
  # @param[in] renderer Markdown renderer.
  # @param[in] tokens Markdown tokens.
  # @param[in] index Current token index.
  # @param[in] options Renderer options.
  # @param[in] environment Render environment.
  # @return Rendered closing HTML.
  ##
  return renderer.renderToken(tokens, index, options, environment) + "</div>\n"


def paragraph_contains_only_markdown_image(tokens, index):
  ##
  # @brief Determine whether a paragraph contains only one Markdown image.
  # @param[in] tokens Markdown tokens.
  # @param[in] index Paragraph-opening token index.
  # @return True for an image-only paragraph.
  ##
  if (
    index + 2 >= len(tokens)
    or tokens[index + 1].type != "inline"
    or tokens[index + 2].type != "paragraph_close"
  ):
    return False
  children = tokens[index + 1].children or []
  types = [child.type for child in children]
  return types in (["image"], ["link_open", "image", "link_close"])


def render_paragraph_open(renderer, tokens, index, options, environment):
  ##
  # @brief Render image-only paragraphs with their alignment class.
  # @param[in] renderer Markdown renderer.
  # @param[in] tokens Markdown tokens.
  # @param[in] index Current token index.
  # @param[in] options Renderer options.
  # @param[in] environment Render environment.
  # @return Rendered opening HTML.
  ##
  if paragraph_contains_only_markdown_image(tokens, index):
    return '<p class="markdown-image-block">\n'
  return renderer.renderToken(tokens, index, options, environment)


def has_highlightable_markdown_fence(renderer, content):
  ##
  # @brief Determine whether Markdown requires syntax highlighting.
  # @param[in] renderer Markdown renderer.
  # @param[in] content Markdown content.
  # @return True for a non-Mermaid language fence.
  ##
  for token in renderer.parse(content):
    if token.type != "fence":
      continue
    language = token.info.strip().split(maxsplit=1)
    if language and language[0].lower() != "mermaid":
      return True
  return False


def render_page(
  template,
  current_page,
  navigation,
  test_results_navigation=None,
  **context,
):
  ##
  # @brief Render a generated page with its shared navigation context.
  # @param[in] template Jinja page template.
  # @param[in] current_page Generated page path.
  # @param[in] navigation Repository navigation entries.
  # @param[in] test_results_navigation Test-result navigation entries.
  # @param[in] context Template-specific context.
  # @return Rendered HTML page.
  ##
  return template.render(
    current_page=current_page,
    href_between=href_between,
    is_navigation_ancestor=is_navigation_ancestor,
    home_href=href_between(current_page, Path("index.html")),
    navigation=navigation,
    test_results_navigation=test_results_navigation or [],
    stylesheet_href=href_between(current_page, STYLESHEET_DESTINATION),
    highlight_href=href_between(current_page, HIGHLIGHT_DESTINATION),
    mermaid_href=href_between(current_page, MERMAID_DESTINATION),
    script_href=href_between(current_page, SCRIPT_DESTINATION),
    **context,
  )


def doxyfile_quote(value):
  ##
  # @brief Quote a Doxygen configuration value without changing its meaning.
  # @param[in] value Configuration value.
  # @return Doxygen-quoted value.
  ##
  return '"' + str(value).replace("\\", "\\\\").replace('"', '\\"') + '"'


def doxyfile_input_paths(root, paths):
  ##
  # @brief Return Doxygen INPUT entries for selected API sources.
  # @param[in] root Repository root.
  # @param[in] paths Selected API source paths.
  # @return Doxygen INPUT value.
  ##
  quoted = [doxyfile_quote(root / path) for path in sorted(paths)]
  return " \\\n                         ".join(quoted)


def materialize_doxyfile(root, output, project_title, source_paths):
  ##
  # @brief Write a Doxygen configuration for selected API source files.
  # @param[in] root Repository root.
  # @param[in] output Generated output path.
  # @param[in] project_title Project title.
  # @param[in] source_paths Selected API source paths.
  # @return Doxyfile and Doxygen output paths.
  ##
  template = Path(__file__).with_name("Doxyfile.in").read_text(encoding="utf-8")
  doxygen_dir = output / "doxygen"
  doxyfile_title = project_title.replace("\n", " ").replace("\r", " ")
  config = (
    template.replace("@INPUT_DIRECTORY@", doxyfile_quote(root))
    .replace("@INPUT_FILES@", doxyfile_input_paths(root, source_paths))
    .replace("@OUTPUT_DIRECTORY@", doxyfile_quote(doxygen_dir))
    .replace("@WARNING_LOG@", doxyfile_quote(doxygen_dir / "warnings.log"))
    .replace("@PROJECT_NAME@", doxyfile_quote(doxyfile_title)[1:-1])
  )
  config_path = doxygen_dir / "Doxyfile"
  config_path.parent.mkdir(parents=True, exist_ok=True)
  config_path.write_text(config, encoding="utf-8")
  return (
    config_path,
    doxygen_dir / "html",
    doxygen_dir / "xml",
    doxygen_dir / "warnings.log",
  )


def index_source_path(name, root, source_paths):
  ##
  # @brief Resolve a Doxygen file identity to a selected source path.
  # @param[in] name Doxygen-reported file identity.
  # @param[in] root Repository root.
  # @param[in] source_paths Selected source paths.
  # @return Selected source path or None.
  ##
  normalized = name.replace("\\", "/")
  candidate = Path(normalized)
  if candidate.is_absolute():
    try:
      normalized = relative_path(candidate.resolve(), root)
    except ValueError:
      return None
  normalized = posixpath.normpath(normalized).removeprefix("./")
  if normalized.startswith("../") or normalized == "..":
    return None
  return normalized if normalized in source_paths else None


def compound_source_path(xml_dir, refid, root, source_paths):
  ##
  # @brief Return the selected source location reported by a file compound.
  # @param[in] xml_dir Doxygen XML directory.
  # @param[in] refid Doxygen compound identifier.
  # @param[in] root Repository root.
  # @param[in] source_paths Selected source paths.
  # @return Selected source path or None.
  ##
  compound_path = xml_dir / f"{refid}.xml"
  if not compound_path.is_file():
    return None
  try:
    compound = element_tree.parse(compound_path).getroot().find("compounddef")
  except (OSError, element_tree.ParseError):
    return None
  if (
    compound is None
    or compound.get("id") != refid
    or compound.get("kind") != "file"
  ):
    return None
  location = compound.find("location")
  location_file = location.get("file") if location is not None else None
  if not location_file:
    return None
  return index_source_path(location_file, root, source_paths)


def collect_api_mapping(xml_dir, html_dir, root, source_paths):
  ##
  # @brief Map selected sources to native Doxygen file pages.
  # @param[in] xml_dir Doxygen XML directory.
  # @param[in] html_dir Doxygen HTML directory.
  # @param[in] root Repository root.
  # @param[in] source_paths Selected source paths.
  # @return Source associations and mapping warnings.
  ##
  index = xml_dir / "index.xml"
  if not index.is_file():
    raise DocgenError("Doxygen did not produce xml/index.xml")
  try:
    index_root = element_tree.parse(index).getroot()
  except element_tree.ParseError as error:
    raise DocgenError(f"cannot parse Doxygen index: {error}") from error
  associations = {}
  warnings = []
  for compound in index_root.findall("compound"):
    if compound.get("kind") != "file":
      continue
    refid = compound.get("refid")
    name = compound.findtext("name", "")
    if not refid or Path(refid).name != refid:
      warnings.append(f"Doxygen index has an unsafe file identifier: {refid!r}")
      continue
    source_path = compound_source_path(xml_dir, refid, root, source_paths)
    if source_path is None:
      warnings.append(
        f"Doxygen file compound has no exact mirrored source location: {name}"
      )
      continue
    if Path(source_path).suffix.lower() in ASSEMBLY_SUFFIXES:
      warnings.append(
        f"Doxygen index file compound is excluded assembly: {name}"
      )
      continue
    html_page = html_dir / f"{refid}.html"
    if not html_page.is_file():
      raise DocgenError(f"Doxygen HTML file page missing: {html_page}")
    if source_path in associations:
      warnings.append(
        f"Doxygen index has multiple file compounds for: {source_path}"
      )
      continue
    associations[source_path] = refid
  return associations, warnings


def run_doxygen(root, output, project_title, source_paths):
  ##
  # @brief Run Doxygen and collect native API-page associations.
  # @param[in] root Repository root.
  # @param[in] output Generated output path.
  # @param[in] project_title Project title.
  # @param[in] source_paths Selected source paths.
  # @return Source associations and Doxygen warnings.
  ##
  source_paths = {
    path
    for path in source_paths
    if Path(path).suffix.lower() in DOXYGEN_SUFFIXES
  }
  if not source_paths:
    return {}, []
  config, html_dir, xml_dir, warning_log = materialize_doxyfile(
    root, output, project_title, source_paths
  )
  process = subprocess.run(
    ["doxygen", str(config)],
    check=False,
    text=True,
    capture_output=True,
  )
  if process.returncode:
    raise DocgenError("Doxygen failed:\n" + process.stdout + process.stderr)
  warnings = (
    warning_log.read_text(encoding="utf-8", errors="replace").splitlines()
    if warning_log.exists()
    else []
  )
  associations, mapping_warnings = collect_api_mapping(
    xml_dir, html_dir, root, source_paths
  )
  return associations, warnings + mapping_warnings


def main():
  ##
  # @brief Generate repository documentation from command-line options.
  ##
  args = parse_args()
  root = args.root.resolve()
  output = args.output if args.output.is_absolute() else root / args.output
  output = output.resolve()
  if not root.is_dir():
    raise DocgenError(f"repository root does not exist: {root}")
  if output == root or root not in output.parents:
    raise DocgenError(
      "--output must be a child of --root to prevent unsafe deletion"
    )

  # Validate external logs before replacing any existing documentation.
  test_results = validate_test_results(args.test_result)
  if output.exists():
    shutil.rmtree(output)

  # Seed the output tree with frontend assets shared by every generated page.
  stylesheet_target = output / STYLESHEET_DESTINATION
  stylesheet_target.parent.mkdir(parents=True, exist_ok=True)
  shutil.copy2(STYLESHEET_SOURCE, stylesheet_target)
  shutil.copy2(SCRIPT_SOURCE, output / SCRIPT_DESTINATION)
  shutil.copy2(MERMAID_SOURCE, output / MERMAID_DESTINATION)
  shutil.copy2(MERMAID_LICENSE_SOURCE, output / MERMAID_LICENSE_DESTINATION)
  shutil.copy2(HIGHLIGHT_SOURCE, output / HIGHLIGHT_DESTINATION)
  shutil.copy2(HIGHLIGHT_LICENSE_SOURCE, output / HIGHLIGHT_LICENSE_DESTINATION)
  templates = create_template_environment()
  project_title = resolve_project_title(root, args.title)

  # Discover sources only after the output tree is safe to exclude.
  files, excluded, symlink_warnings = discover_files(
    root, output, set(args.exclude), args.include_ignored
  )
  if not files:
    raise DocgenError("no files selected for documentation generation")
  contents = {relative_path(path, root): read_text(path) for path in files}

  # Establish all mirrored destinations before rewriting cross-file links.
  resource_destinations = {}
  page_destinations = {}
  for rel, content in contents.items():
    resource_destinations[rel] = (
      page_path(rel) if is_text_file(Path(rel), content) else asset_path(rel)
    )
    page_destinations[rel] = page_path(rel)
  directory_destinations = {".": Path("index.html")}
  for rel in contents:
    directory = Path(rel).parent
    while directory != Path("."):
      directory_destinations.setdefault(
        directory.as_posix(), directory_page_path(directory)
      )
      directory = directory.parent
  resource_destinations.update(directory_destinations)
  associations, doxy_warnings = {}, []

  # Generate native API output before linking its Docgen wrapper pages.
  if not args.no_api:
    associations, doxy_warnings = run_doxygen(
      root, output, project_title, set(contents)
    )
  file_paths = [Path(key) for key in contents]
  api_destinations = {
    rel: Path("api") / "files" / (rel + ".html")
    for rel in associations
    if rel in contents
  }
  for index, test_result in enumerate(test_results, start=1):
    test_result["destination"] = Path("test-results") / f"{index}.html"
  test_results_navigation = test_results
  navigation = build_navigation(
    file_paths, page_destinations, directory_destinations, api_destinations
  )
  markdown_renderer = create_markdown_renderer()
  link_warnings = []

  # Render source pages after the complete link destination map is available.
  for rel, content in contents.items():
    source = root / rel
    resource_destination = resource_destinations[rel]
    destination = page_destinations[rel]
    if resource_destination.parts[0] == "assets":
      asset_target = output / resource_destination
      asset_target.parent.mkdir(parents=True, exist_ok=True)
      shutil.copy2(source, asset_target)
    target = output / destination
    target.parent.mkdir(parents=True, exist_ok=True)
    related_api = (
      {
        "href": href_between(destination, api_destinations[rel]),
        "name": "Open API documentation",
      }
      if rel in api_destinations
      else None
    )

    if Path(rel).suffix.lower() in MARKDOWN_SUFFIXES:
      rendered = markdown_renderer.render(content or "")
      markdown_content = Markup(
        rewrite_html_links(
          rendered, rel, destination, resource_destinations, link_warnings
        )
      )
      markdown_highlighting = has_highlightable_markdown_fence(
        markdown_renderer, content or ""
      )
    else:
      markdown_content = None
      markdown_highlighting = False
    target.write_text(
      render_page(
        templates.get_template("source.html.j2"),
        destination,
        navigation,
        test_results_navigation=test_results_navigation,
        title=rel,
        project_title=project_title,
        path=rel,
        text=content or "",
        raw_language=(
          raw_language(Path(rel))
          if content is not None
          and Path(rel).suffix.lower() not in MARKDOWN_SUFFIXES
          else None
        ),
        markdown_highlighting=markdown_highlighting,
        markdown_content=markdown_content,
        is_binary=not is_text_file(Path(rel), content),
        asset_href=(
          href_between(destination, resource_destination)
          if resource_destination != destination
          else None
        ),
        related_api=related_api,
      ),
      encoding="utf-8",
    )

  # Render a navigable index for every selected non-root directory.
  for directory in directory_entries(navigation):
    destination = directory["destination"]
    target = output / destination
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(
      render_page(
        templates.get_template("directory.html.j2"),
        destination,
        navigation,
        test_results_navigation=test_results_navigation,
        title=directory["path"],
        project_title=project_title,
        path=directory["path"],
        entries=directory["children"],
      ),
      encoding="utf-8",
    )

  # Add wrappers only for Doxygen pages associated with selected sources.
  for rel, destination in sorted(api_destinations.items()):
    refid = associations[rel]
    target = output / destination
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(
      render_page(
        templates.get_template("api.html.j2"),
        destination,
        navigation,
        test_results_navigation=test_results_navigation,
        title=f"API: {rel}",
        project_title=project_title,
        path=rel,
        raw_source_href=href_between(destination, page_destinations[rel]),
        doxygen_href=href_between(
          destination, Path("doxygen") / "html" / f"{refid}.html"
        ),
        page_kind="api-page",
      ),
      encoding="utf-8",
    )

  # Render each attached log as an independently navigable test-result page.
  for test_result in test_results:
    destination = test_result["destination"]
    target = output / destination
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(
      render_page(
        templates.get_template("test-result.html.j2"),
        destination,
        navigation,
        test_results_navigation=test_results_navigation,
        title=f"{test_result['label']} - Test results",
        project_title=project_title,
        test_result=test_result,
      ),
      encoding="utf-8",
    )

  # Rebase the root README's local links against the generated index page.
  index_page = Path("index.html")
  readme = root_readme(contents)
  readme_content = None
  if readme is not None:
    rendered = markdown_renderer.render(contents[readme] or "")
    readme_content = Markup(
      rewrite_html_links(
        rendered,
        readme,
        index_page,
        resource_destinations,
        link_warnings,
      )
    )
  readme_highlighting = (
    has_highlightable_markdown_fence(markdown_renderer, contents[readme] or "")
    if readme is not None
    else False
  )

  # Collect warnings only after all Markdown link rewriting is complete.
  all_warnings = (
    symlink_warnings
    + [f"Doxygen: {warning}" for warning in doxy_warnings]
    + link_warnings
  )
  excluded_frontier = exclusion_frontier(
    [relative_path(path, root) for path in files], excluded
  )
  warning_text = "\n".join(all_warnings) + ("\n" if all_warnings else "")
  (output / "warnings.txt").write_text(warning_text, encoding="utf-8")
  (output / "index.html").write_text(
    render_page(
      templates.get_template("index.html.j2"),
      index_page,
      navigation,
      test_results_navigation=test_results_navigation,
      title=project_title,
      project_title=project_title,
      file_count=len(files),
      api_count=len(associations),
      warnings=all_warnings,
      excluded=excluded_frontier,
      markdown_highlighting=readme_highlighting,
      readme_content=readme_content,
      readme_path=readme,
    ),
    encoding="utf-8",
  )
  print(
    f"Documentation generated: {len(files)} files, "
    f"{len(associations)} API pages, "
    f"{len(all_warnings)} warnings: {output}"
  )
  if args.strict and all_warnings:
    raise DocgenError(
      "strict mode rejects documentation warnings; see warnings.txt"
    )


# ==============================================================================
# SCRIPT ENTRY POINT
# ==============================================================================

if __name__ == "__main__":
  try:
    main()
  except (DocgenError, OSError) as error:
    print(f"docgen: error: {error}", file=sys.stderr)
    sys.exit(1)
