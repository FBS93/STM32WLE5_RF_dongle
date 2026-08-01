"""Black-box tests for Docgen input selection and safety boundaries."""

import subprocess
import sys

from conftest import DOCGEN_PATH
from conftest import run_docgen


def test_uses_default_output_and_reports_invalid_input_without_replacing_output(
  tmp_path,
):
  repository_root = tmp_path / "repository"
  repository_root.mkdir()
  (repository_root / "README.md").write_text("# Overview\n", encoding="utf-8")

  result = subprocess.run(
    [sys.executable, str(DOCGEN_PATH), "--root", ".", "--no-api"],
    check=False,
    cwd=repository_root,
    text=True,
    capture_output=True,
  )

  assert result.returncode == 0, result.stderr
  assert (repository_root / "build/docgen/index.html").is_file()

  output_path = repository_root / "titled-report"
  result = run_docgen(
    repository_root, output_path, "--no-api", "--title", "Explicit title"
  )
  assert result.returncode == 0, result.stderr
  assert "Explicit title" in (output_path / "index.html").read_text(
    encoding="utf-8"
  )

  output_path = repository_root / "report"
  output_path.mkdir()
  sentinel = output_path / "previous.txt"
  sentinel.write_text("keep\n", encoding="utf-8")
  invalid_log = tmp_path / "invalid.log"
  invalid_log.write_bytes(b"\xff")
  result = run_docgen(
    repository_root,
    output_path,
    "--no-api",
    "--test-result",
    "Invalid",
    str(invalid_log),
  )
  assert result.returncode == 1
  assert "test result is not valid UTF-8" in result.stderr
  assert sentinel.read_text(encoding="utf-8") == "keep\n"


def test_applies_gitignore_and_keeps_git_and_output_excluded(tmp_path):
  repository_root = tmp_path / "repository"
  nested = repository_root / "nested"
  nested.mkdir(parents=True)
  (repository_root / ".git").mkdir()
  (repository_root / ".gitignore").write_text(
    "*.private\n!published.private\nignored-dir/\n", encoding="utf-8"
  )
  (nested / ".gitignore").write_text("nested-hidden.txt\n", encoding="utf-8")
  (repository_root / "hidden.private").write_text("hidden\n", encoding="utf-8")
  (repository_root / "published.private").write_text(
    "published\n", encoding="utf-8"
  )
  (repository_root / "ignored-dir").mkdir()
  (repository_root / "ignored-dir" / "hidden.txt").write_text(
    "hidden\n", encoding="utf-8"
  )
  (nested / "nested-hidden.txt").write_text("hidden\n", encoding="utf-8")
  (nested / "visible.txt").write_text("visible\n", encoding="utf-8")
  (repository_root / ".git" / "git-only.txt").write_text(
    "hidden\n", encoding="utf-8"
  )
  output_path = repository_root / "report"

  result = run_docgen(repository_root, output_path, "--no-api")

  assert result.returncode == 0, result.stderr
  index = (output_path / "index.html").read_text(encoding="utf-8")
  assert not (output_path / "files/hidden.private.html").exists()
  assert "published.private" in index
  assert not (output_path / "files/ignored-dir/index.html").exists()
  assert not (output_path / "files/nested/nested-hidden.txt.html").exists()
  assert "visible.txt" in index
  assert not (output_path / "files/.git/git-only.txt.html").exists()
  assert "report/" not in index

  result = run_docgen(
    repository_root, output_path, "--no-api", "--include-ignored"
  )
  assert result.returncode == 0, result.stderr
  index = (output_path / "index.html").read_text(encoding="utf-8")
  assert "hidden.private" in index
  assert "hidden.txt" in index
  assert "git-only.txt" not in index


def test_prunes_fully_ignored_directory_and_reports_its_root(tmp_path):
  repository_root = tmp_path / "repository"
  ignored_directory = repository_root / "generated" / "nested"
  ignored_directory.mkdir(parents=True)
  (repository_root / "README.md").write_text("# Overview\n", encoding="utf-8")
  (repository_root / ".gitignore").write_text("generated/\n", encoding="utf-8")
  (ignored_directory / "hidden.txt").write_text("hidden\n", encoding="utf-8")
  output_path = repository_root / "report"

  result = run_docgen(repository_root, output_path, "--no-api")

  assert result.returncode == 0, result.stderr
  assert "generated/" not in (output_path / "index.html").read_text(
    encoding="utf-8"
  )
  index = (output_path / "index.html").read_text(encoding="utf-8")
  assert "generated - .gitignore" in index
  assert "generated/nested" not in index


def test_keeps_partially_ignored_directory_with_included_child(tmp_path):
  repository_root = tmp_path / "repository"
  ignored_directory = repository_root / "generated"
  ignored_directory.mkdir(parents=True)
  (repository_root / "README.md").write_text("# Overview\n", encoding="utf-8")
  (repository_root / ".gitignore").write_text(
    "generated/*.tmp\n!generated/published.tmp\n", encoding="utf-8"
  )
  (ignored_directory / "hidden.tmp").write_text("hidden\n", encoding="utf-8")
  (ignored_directory / "published.tmp").write_text(
    "published\n", encoding="utf-8"
  )
  output_path = repository_root / "report"

  result = run_docgen(repository_root, output_path, "--no-api")

  assert result.returncode == 0, result.stderr
  generated_page = (output_path / "files/generated/index.html").read_text(
    encoding="utf-8"
  )
  assert "hidden.tmp" not in generated_page
  assert "published.tmp" in generated_page


def test_excludes_requested_directory_and_reports_it(tmp_path):
  repository_root = tmp_path / "repository"
  excluded = repository_root / "generated" / "nested"
  excluded.mkdir(parents=True)
  (repository_root / "README.md").write_text("# Overview\n", encoding="utf-8")
  (repository_root / "included.txt").write_text("included\n", encoding="utf-8")
  (excluded / "hidden.txt").write_text("hidden\n", encoding="utf-8")
  output_path = repository_root / "report"

  result = run_docgen(
    repository_root, output_path, "--no-api", "--exclude", "generated"
  )

  assert result.returncode == 0, result.stderr
  index = (output_path / "index.html").read_text(encoding="utf-8")
  assert "included.txt" in index
  assert "generated/" not in index
  assert "generated - user exclusion" in index


def test_mirrors_internal_symlinks_and_rejects_external_targets(tmp_path):
  repository_root = tmp_path / "repository"
  repository_root.mkdir()
  external_file = tmp_path / "outside.txt"
  external_file.write_text("must not be mirrored\n", encoding="utf-8")
  (repository_root / "inside.txt").write_text("inside\n", encoding="utf-8")
  (repository_root / "inside-link.txt").symlink_to("inside.txt")
  (repository_root / "outside-link.txt").symlink_to(external_file)
  output_path = repository_root / "report"

  result = run_docgen(repository_root, output_path, "--no-api")

  assert result.returncode == 0, result.stderr
  index = (output_path / "index.html").read_text(encoding="utf-8")
  assert "inside-link.txt" in index
  assert not (output_path / "files/outside-link.txt.html").exists()
  assert "outside-link.txt: symlink target outside --root" in (
    output_path / "warnings.txt"
  ).read_text(encoding="utf-8")
