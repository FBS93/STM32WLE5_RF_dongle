#!/usr/bin/env python3

# ==============================================================================
# @brief Synchronize third-party GitHub repositories by snapshot reference.
#
# This script reads a manifest JSON file with repository entries and keeps local
# workspace snapshots updated to resolved GitHub references.
#
# Optional environment variable:
#   GH_TOKEN Personal access token for GitHub API requests.
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
import json
import os
import shutil
import sys
import tarfile
import tempfile
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path

# ------------------------------------------------------------------------------
# External library imports
# ------------------------------------------------------------------------------
from jsonschema import Draft202012Validator
from jsonschema import exceptions as jsonschema_exceptions

# ------------------------------------------------------------------------------
# Project-specific imports
# ------------------------------------------------------------------------------

# ==============================================================================
# CONSTANTS
# ==============================================================================

GITHUB_API_BASE_URL = "https://api.github.com"

##
# @brief JSON Schema path for repository manifest validation.
##
MANIFEST_SCHEMA_PATH = Path(__file__).with_name("repos.schema.json")

# ==============================================================================
# CLASSES
# ==============================================================================

# ==============================================================================
# FUNCTIONS
# ==============================================================================


##
# @brief Parse command-line arguments.
#
# @return Parsed arguments namespace.
##
def parse_args():
  description = (
    "Synchronize third-party GitHub repositories by snapshot reference."
  )
  parser = argparse.ArgumentParser(description=description)
  parser.add_argument(
    "--repository-id",
    help="Synchronize only the repository entry matching this id.",
  )
  parser.add_argument(
    "--manifest",
    default="tools/third_party_sync/repos.json",
    help="Path to manifest file.",
  )
  parser.add_argument(
    "--lock",
    default="tools/third_party_sync/repos.lock.json",
    help="Path to lock file.",
  )
  parser.add_argument(
    "--workspace",
    default=".",
    help="Workspace root used to resolve repository paths.",
  )
  parser.add_argument(
    "--dry-run",
    action="store_true",
    help="Show updates without writing files.",
  )

  return parser.parse_args()


##
# @brief Read and parse a JSON file.
#
# @param[in] file_path Absolute path to the JSON file.
# @param[in] required If True, fails when file does not exist.
# @return Parsed JSON object.
##
def read_json_file(file_path, required):
  if not file_path.exists():
    if required:
      print(f"❌ Missing required file: {file_path}", flush=True)
      sys.exit(1)
    return {}

  try:
    with file_path.open("r", encoding="utf-8") as file_handle:
      return json.load(file_handle)
  except json.JSONDecodeError as error:
    print(f"❌ Invalid JSON in {file_path}: {error}", flush=True)
    sys.exit(1)


##
# @brief Format a JSON Schema validation path for diagnostics.
#
# @param[in] path_elements JSON Schema validation path elements.
# @return Human-readable dotted path.
##
def format_schema_path(path_elements):
  formatted_path = "$"

  for path_element in path_elements:
    if isinstance(path_element, int):
      formatted_path += f"[{path_element}]"
    else:
      formatted_path += f".{path_element}"

  return formatted_path


##
# @brief Validate a JSON instance against a JSON Schema file.
#
# @param[in] instance Parsed JSON instance.
# @param[in] schema_file JSON Schema file path.
# @param[in] source_file Source JSON file path for diagnostics.
##
def validate_json_schema(instance, schema_file, source_file):
  schema = read_json_file(schema_file, required=True)

  try:
    Draft202012Validator.check_schema(schema)
    validator = Draft202012Validator(schema)
    validator.validate(instance)
  except jsonschema_exceptions.SchemaError as error:
    schema_path = format_schema_path(error.schema_path)
    print(
      f"❌ Invalid JSON Schema in {schema_file} at {schema_path}: "
      f"{error.message}",
      flush=True,
    )
    sys.exit(1)
  except jsonschema_exceptions.ValidationError as error:
    instance_path = format_schema_path(error.path)
    print(
      f"❌ Invalid manifest in {source_file} at {instance_path}: "
      f"{error.message}",
      flush=True,
    )
    sys.exit(1)


##
# @brief Write a JSON object to disk.
#
# @param[in] file_path Absolute path to the JSON file.
# @param[in] payload JSON payload to serialize.
##
def write_json_file(file_path, payload):
  file_path.parent.mkdir(parents=True, exist_ok=True)
  with file_path.open("w", encoding="utf-8") as file_handle:
    json.dump(payload, file_handle, indent=2, sort_keys=True)
    file_handle.write("\n")


##
# @brief Validate manifest structure and return repository entries.
#
# @param[in] manifest Manifest JSON object.
# @return List of validated repository entries.
##
def validate_manifest(manifest):
  repositories = manifest.get("repositories")

  seen_ids = set()
  validated = []

  for index, repository in enumerate(repositories, start=1):
    repository_id = repository.get("id")
    repository_url = repository.get("url")
    repository_path = repository.get("path")
    repository_ref_type = repository.get("ref_type", "auto")
    repository_ref = repository.get("ref", "")

    if repository_id in seen_ids:
      print(f"❌ Duplicate repository id: {repository_id}", flush=True)
      sys.exit(1)
    seen_ids.add(repository_id)

    if Path(repository_path).is_absolute():
      print(
        f"❌ repositories[{index}].path must be workspace-relative.",
        flush=True,
      )
      sys.exit(1)

    validated.append(
      {
        "id": repository_id,
        "url": repository_url,
        "path": repository_path,
        "ref_type": repository_ref_type,
        "ref": repository_ref,
      }
    )

  return validated


##
# @brief Filter repositories by id when requested.
#
# @param[in] repositories Validated repository entries.
# @param[in] repository_id Optional repository id filter.
# @return Filtered repository entries.
##
def filter_repositories(repositories, repository_id):
  if repository_id is None:
    return repositories

  filtered_repositories = [
    repository
    for repository in repositories
    if repository["id"] == repository_id
  ]

  if len(filtered_repositories) == 0:
    print(
      f"❌ Repository id not found in manifest: {repository_id}", flush=True
    )
    sys.exit(1)

  return filtered_repositories


##
# @brief Parse owner/repo from a GitHub URL.
#
# @param[in] repository_url GitHub repository URL.
# @return Repository slug in the format owner/repo.
##
def parse_github_slug(repository_url):
  parsed = urllib.parse.urlparse(repository_url)

  if parsed.netloc not in ["github.com", "www.github.com"]:
    print(
      f"❌ Only github.com URLs are supported: {repository_url}", flush=True
    )
    sys.exit(1)

  path = parsed.path.rstrip("/")
  if path.endswith(".git"):
    path = path[:-4]

  segments = [segment for segment in path.split("/") if segment]
  if len(segments) < 2:
    print(f"❌ Invalid GitHub repository URL: {repository_url}", flush=True)
    sys.exit(1)

  return f"{segments[0]}/{segments[1]}"


##
# @brief Perform a GitHub API GET request and decode JSON response.
#
# @param[in] url Fully qualified GitHub API URL.
# @param[in] github_token Optional GitHub token.
# @return Parsed JSON response.
##
def github_api_get_json(url, github_token):
  headers = {
    "Accept": "application/vnd.github+json",
    "User-Agent": "embedded-workbench-third-party-sync",
  }

  if github_token:
    headers["Authorization"] = f"Bearer {github_token}"

  request = urllib.request.Request(url, headers=headers, method="GET")

  try:
    with urllib.request.urlopen(request) as response:
      payload = response.read().decode("utf-8")
      return json.loads(payload)
  except urllib.error.HTTPError as error:
    error_message = error.read().decode("utf-8", errors="replace")
    print(
      f"❌ GitHub API request failed ({error.code}) for {url}: {error_message}",
      flush=True,
    )
    sys.exit(1)
  except urllib.error.URLError as error:
    print(f"❌ Network error while requesting {url}: {error}", flush=True)
    sys.exit(1)


##
# @brief Perform a GitHub API GET request returning None on HTTP 404.
#
# @param[in] url Fully qualified GitHub API URL.
# @param[in] github_token Optional GitHub token.
# @return Parsed JSON response, or None when HTTP status is 404.
##
def github_api_get_json_or_none(url, github_token):
  headers = {
    "Accept": "application/vnd.github+json",
    "User-Agent": "embedded-workbench-third-party-sync",
  }

  if github_token:
    headers["Authorization"] = f"Bearer {github_token}"

  request = urllib.request.Request(url, headers=headers, method="GET")

  try:
    with urllib.request.urlopen(request) as response:
      payload = response.read().decode("utf-8")
      return json.loads(payload)
  except urllib.error.HTTPError as error:
    if error.code == 404:
      return None

    error_message = error.read().decode("utf-8", errors="replace")
    print(
      f"❌ GitHub API request failed ({error.code}) for {url}: {error_message}",
      flush=True,
    )
    sys.exit(1)
  except urllib.error.URLError as error:
    print(f"❌ Network error while requesting {url}: {error}", flush=True)
    sys.exit(1)


##
# @brief Resolve the commit SHA associated with a repository ref.
#
# @param[in] repository_slug Repository slug in owner/repo format.
# @param[in] ref_name Git ref name to resolve.
# @param[in] github_token Optional GitHub token.
# @return Commit SHA.
##
def resolve_commit_sha(repository_slug, ref_name, github_token, error_context):
  encoded_ref_name = urllib.parse.quote(ref_name, safe="")
  commit_api_url = (
    f"{GITHUB_API_BASE_URL}/repos/{repository_slug}/commits/{encoded_ref_name}"
  )

  headers = {
    "Accept": "application/vnd.github+json",
    "User-Agent": "embedded-workbench-third-party-sync",
  }

  if github_token:
    headers["Authorization"] = f"Bearer {github_token}"

  request = urllib.request.Request(
    commit_api_url, headers=headers, method="GET"
  )

  try:
    with urllib.request.urlopen(request) as response:
      payload = response.read().decode("utf-8")
      commit_metadata = json.loads(payload)
  except urllib.error.HTTPError as error:
    if error.code in [404, 422]:
      print(
        f"❌ {error_context} not found in {repository_slug}: '{ref_name}'",
        flush=True,
      )
      sys.exit(1)

    error_message = error.read().decode("utf-8", errors="replace")
    print(
      f"❌ GitHub API request failed ({error.code}) for "
      f"{commit_api_url}: {error_message}",
      flush=True,
    )
    sys.exit(1)
  except urllib.error.URLError as error:
    print(
      f"❌ Network error while requesting {commit_api_url}: {error}", flush=True
    )
    sys.exit(1)

  commit_sha = commit_metadata.get("sha")

  if not isinstance(commit_sha, str) or commit_sha.strip() == "":
    print(
      f"❌ Missing commit SHA for ref '{ref_name}' in {repository_slug}",
      flush=True,
    )
    sys.exit(1)

  return commit_sha


##
# @brief Resolve snapshot metadata for one repository entry.
#
# Resolution order:
# - auto + empty ref: latest release, fallback to default branch HEAD.
# - release + empty ref: latest release.
# - release + non-empty ref: pinned release.
# - commit + empty ref: default branch HEAD.
# - commit + non-empty ref: pinned commit.
#
# @param[in] repository_slug Repository slug in owner/repo format.
# @param[in] ref_type Snapshot reference type.
# @param[in] ref_value Snapshot reference value.
# @param[in] github_token Optional GitHub token.
# @return Snapshot metadata dictionary.
##
def resolve_snapshot(repository_slug, ref_type, ref_value, github_token):
  ref_value = ref_value.strip()

  def resolve_latest_release_or_none():
    release_api_url = (
      f"{GITHUB_API_BASE_URL}/repos/{repository_slug}/releases/latest"
    )
    return github_api_get_json_or_none(release_api_url, github_token)

  def resolve_release_by_tag_or_fail(release_tag):
    encoded_tag = urllib.parse.quote(release_tag, safe="")
    release_api_url = (
      f"{GITHUB_API_BASE_URL}/repos/{repository_slug}/releases/tags/"
      f"{encoded_tag}"
    )
    metadata = github_api_get_json_or_none(release_api_url, github_token)
    if metadata is None:
      print(
        f"❌ Release not found in {repository_slug}: '{release_tag}'",
        flush=True,
      )
      sys.exit(1)
    return metadata

  release_metadata = None
  if ref_type == "release":
    if ref_value == "":
      release_metadata = resolve_latest_release_or_none()
      if release_metadata is None:
        print(
          f"❌ No releases found in {repository_slug}.",
          flush=True,
        )
        sys.exit(1)
    else:
      release_metadata = resolve_release_by_tag_or_fail(ref_value)
  elif ref_type == "auto":
    if ref_value == "":
      release_metadata = resolve_latest_release_or_none()
    else:
      release_metadata = resolve_release_by_tag_or_fail(ref_value)

  if isinstance(release_metadata, dict):
    resolved_release = release_metadata.get("tag_name")
    published_at = release_metadata.get("published_at")
    tarball_url = release_metadata.get("tarball_url")

    if not isinstance(resolved_release, str) or resolved_release.strip() == "":
      print(
        f"❌ Missing tag_name in release metadata for {repository_slug}",
        flush=True,
      )
      sys.exit(1)

    if not isinstance(tarball_url, str) or tarball_url.strip() == "":
      print(
        f"❌ Missing tarball_url in release metadata for {repository_slug}",
        flush=True,
      )
      sys.exit(1)

    resolved_commit = resolve_commit_sha(
      repository_slug,
      resolved_release,
      github_token,
      "Release",
    )

    return {
      "source": "release",
      "resolved_release": resolved_release,
      "resolved_commit": resolved_commit,
      "published_at": published_at,
      "tarball_url": tarball_url,
    }

  repository_api_url = f"{GITHUB_API_BASE_URL}/repos/{repository_slug}"
  repository_metadata = github_api_get_json(repository_api_url, github_token)
  default_branch = repository_metadata.get("default_branch")

  if not isinstance(default_branch, str) or default_branch.strip() == "":
    print(
      f"❌ Missing default_branch for repository {repository_slug}", flush=True
    )
    sys.exit(1)

  if ref_type == "commit" and ref_value != "":
    resolved_commit = resolve_commit_sha(
      repository_slug,
      ref_value,
      github_token,
      "Commit",
    )
  else:
    resolved_commit = resolve_commit_sha(
      repository_slug,
      default_branch,
      github_token,
      "Default branch",
    )

  commit_api_url = (
    f"{GITHUB_API_BASE_URL}/repos/{repository_slug}/commits/{resolved_commit}"
  )
  commit_metadata = github_api_get_json(commit_api_url, github_token)

  commit_date = None
  commit_section = commit_metadata.get("commit")
  if isinstance(commit_section, dict):
    committer_section = commit_section.get("committer")
    if isinstance(committer_section, dict):
      commit_date = committer_section.get("date")

  return {
    "source": "default_branch_head",
    "default_branch": default_branch,
    "resolved_commit": resolved_commit,
    "commit_date": commit_date,
    "tarball_url": (
      f"{GITHUB_API_BASE_URL}/repos/{repository_slug}/tarball/{resolved_commit}"
    ),
  }


##
# @brief Download a URL to a local file.
#
# @param[in] url Source URL.
# @param[in] destination_path Local destination file path.
# @param[in] github_token Optional GitHub token.
##
def download_file(url, destination_path, github_token):
  headers = {
    "Accept": "application/vnd.github+json",
    "User-Agent": "embedded-workbench-third-party-sync",
  }

  if github_token:
    headers["Authorization"] = f"Bearer {github_token}"

  request = urllib.request.Request(url, headers=headers, method="GET")

  try:
    with urllib.request.urlopen(request) as response:
      destination_path.write_bytes(response.read())
  except urllib.error.HTTPError as error:
    error_message = error.read().decode("utf-8", errors="replace")
    print(
      f"❌ Download failed ({error.code}) for {url}: {error_message}",
      flush=True,
    )
    sys.exit(1)
  except urllib.error.URLError as error:
    print(f"❌ Network error while downloading {url}: {error}", flush=True)
    sys.exit(1)


##
# @brief Extract a GitHub release tarball into a target path.
#
# This extraction strips the top-level directory generated by GitHub tarballs.
# Symlinks and hardlinks are ignored for safety.
#
# @param[in] tarball_path Path to downloaded tarball.
# @param[in] target_path Destination directory.
##
def extract_tarball_to_path(tarball_path, target_path):
  if target_path.exists():
    shutil.rmtree(target_path)
  target_path.mkdir(parents=True, exist_ok=True)

  with tarfile.open(tarball_path, "r:*") as archive:
    target_root = target_path.resolve()

    for member in archive.getmembers():
      if not member.name:
        continue

      member_name = member.name.lstrip("/")
      member_parts = [
        part for part in member_name.split("/") if part not in ["", "."]
      ]
      if len(member_parts) < 2:
        continue

      relative_parts = member_parts[1:]
      if any(part == ".." for part in relative_parts):
        continue

      destination = target_path.joinpath(*relative_parts)
      destination_parent = destination.parent.resolve()
      if not str(destination_parent).startswith(str(target_root)):
        continue

      if member.isdir():
        destination.mkdir(parents=True, exist_ok=True)
        continue

      if not member.isfile():
        continue

      destination.parent.mkdir(parents=True, exist_ok=True)
      file_object = archive.extractfile(member)
      if file_object is None:
        continue

      with destination.open("wb") as destination_file:
        shutil.copyfileobj(file_object, destination_file)


##
# @brief Determine whether a repository entry needs update.
#
# @param[in] lock_entry Existing lock entry for the repository id.
# @param[in] target_path Destination directory path.
# @param[in] snapshot_metadata Resolved snapshot metadata.
# @return True if update is required, otherwise False.
##
def needs_update(lock_entry, target_path, snapshot_metadata):
  if not target_path.exists():
    return True

  if not isinstance(lock_entry, dict):
    return True

  if lock_entry.get("source") != snapshot_metadata.get("source"):
    return True

  if lock_entry.get("resolved_commit") != snapshot_metadata.get(
    "resolved_commit"
  ):
    return True

  resolved_release = snapshot_metadata.get("resolved_release")
  if isinstance(resolved_release, str) and resolved_release.strip() != "":
    return lock_entry.get("resolved_release") != resolved_release

  return False


##
# @brief Build a readable snapshot label for logs.
#
# @param[in] snapshot_metadata Resolved snapshot metadata.
# @return Human-readable label.
##
def snapshot_label(snapshot_metadata):
  source = snapshot_metadata.get("source")

  if source == "release":
    return snapshot_metadata["resolved_release"]

  if source == "default_branch_head":
    default_branch = snapshot_metadata.get("default_branch", "?")
    resolved_commit = snapshot_metadata.get("resolved_commit", "")
    return f"{default_branch}@{resolved_commit[:12]}"

  return "unknown"


##
# @brief Synchronize repositories from manifest to workspace.
#
# @param[in] repositories Validated repositories list.
# @param[in] lock_data Current lock data.
# @param[in] workspace_root Workspace root path.
# @param[in] dry_run If True, do not write any file.
# @param[in] github_token Optional GitHub token.
# @return Updated lock data.
##
def synchronize_repositories(
  repositories, lock_data, workspace_root, dry_run, github_token
):
  lock_repositories = lock_data.get("repositories")
  if not isinstance(lock_repositories, dict):
    lock_repositories = {}

  updated_lock_repositories = dict(lock_repositories)

  for repository in repositories:
    repository_id = repository["id"]
    repository_slug = parse_github_slug(repository["url"])
    repository_path = workspace_root.joinpath(repository["path"]).resolve()
    repository_ref_type = repository["ref_type"]
    repository_ref = repository["ref"]

    snapshot_metadata = resolve_snapshot(
      repository_slug,
      repository_ref_type,
      repository_ref,
      github_token,
    )

    existing_lock_entry = updated_lock_repositories.get(repository_id)
    repository_needs_update = needs_update(
      existing_lock_entry,
      repository_path,
      snapshot_metadata,
    )

    snapshot_text = snapshot_label(snapshot_metadata)

    if not repository_needs_update:
      print(
        f"✅ {repository_id}: up to date ({snapshot_text})",
        flush=True,
      )
      continue

    print(
      f"⬇️  {repository_id}: syncing {snapshot_text} -> {repository_path}",
      flush=True,
    )

    if not dry_run:
      with tempfile.TemporaryDirectory(
        prefix="third_party_sync_"
      ) as temporary_directory:
        tarball_path = Path(temporary_directory) / "release.tar"
        download_file(
          snapshot_metadata["tarball_url"], tarball_path, github_token
        )
        extract_tarball_to_path(tarball_path, repository_path)

    updated_lock_repositories[repository_id] = snapshot_metadata

  return {"repositories": updated_lock_repositories}


##
# @brief Main third-party synchronization workflow.
#
# Orchestrates:
# - Parse command-line arguments.
# - Load and validate manifest.
# - Load lock file if it exists.
# - Synchronize repository snapshots.
# - Persist lock file changes.
##
def main():
  arguments = parse_args()
  workspace_root = Path(arguments.workspace).resolve()
  manifest_path = Path(arguments.manifest).resolve()
  lock_path = Path(arguments.lock).resolve()

  if not workspace_root.exists() or not workspace_root.is_dir():
    print(f"❌ Invalid workspace directory: {workspace_root}", flush=True)
    sys.exit(1)

  manifest_data = read_json_file(manifest_path, required=True)
  validate_json_schema(manifest_data, MANIFEST_SCHEMA_PATH, manifest_path)
  repositories = validate_manifest(manifest_data)
  repositories = filter_repositories(repositories, arguments.repository_id)

  lock_data = read_json_file(lock_path, required=False)
  github_token = None

  if "GH_TOKEN" in os.environ and os.environ["GH_TOKEN"].strip() != "":
    github_token = os.environ["GH_TOKEN"]

  updated_lock_data = synchronize_repositories(
    repositories,
    lock_data,
    workspace_root,
    arguments.dry_run,
    github_token,
  )

  if arguments.dry_run:
    print("🧪 Dry-run enabled: lock file was not written.", flush=True)
    return

  write_json_file(lock_path, updated_lock_data)
  print(f"✅ Lock file updated: {lock_path}", flush=True)


# ==============================================================================
# SCRIPT ENTRY POINT
# ==============================================================================

if __name__ == "__main__":
  main()
