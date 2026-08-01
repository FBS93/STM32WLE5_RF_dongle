# ==============================================================================
# @brief Common helpers for KiCad state save/restore scripts.
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
import os
import shutil
from pathlib import Path

# ------------------------------------------------------------------------------
# External library imports
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Project-specific imports
# ------------------------------------------------------------------------------

# ==============================================================================
# CONSTANTS
# ==============================================================================

DEFAULT_CONFIG_ROOT = Path.home() / ".config" / "kicad"
DEFAULT_DOCUMENTS_ROOT = Path.home() / ".local" / "share" / "kicad"

# ==============================================================================
# CLASSES
# ==============================================================================

# ==============================================================================
# FUNCTIONS
# ==============================================================================


##
# @brief Return the workspace-local KiCad state root.
#
# @return Absolute path to `tools/kicad_state/state`.
##
def get_state_root_dir():
  return Path(__file__).resolve().parent / "state"


##
# @brief Return the configured KiCad config root.
#
# @return Absolute config root path.
##
def get_env_path(env_name, default_path):
  raw_value = os.environ.get(env_name)
  if raw_value:
    return Path(raw_value)

  return default_path


##
# @brief Return the KiCad config root in the current user home.
#
# @return Absolute config root path.
##
def get_config_root():
  return get_env_path("KICAD_CONFIG_HOME", DEFAULT_CONFIG_ROOT)


##
# @brief Return the KiCad documents root.
#
# @return Absolute documents root path.
##
def get_documents_root():
  return get_env_path("KICAD_DOCUMENTS_HOME", DEFAULT_DOCUMENTS_ROOT)


##
# @brief Return the versioned KiCad documents directory.
#
# @param[in] version KiCad version string.
# @return Absolute versioned documents directory path.
##
def get_versioned_documents_dir(version):
  documents_root = get_documents_root()
  if documents_root.name == "kicad":
    return documents_root / version

  return documents_root / "kicad" / version


##
# @brief Return detected KiCad versions from the config root.
#
# @return Sorted list of version strings.
##
def get_detected_versions():
  config_root = get_config_root()
  if not config_root.exists():
    return []

  return sorted(
    [path.name for path in config_root.iterdir() if path.is_dir()],
    key=parse_version_key,
  )


##
# @brief Convert a dotted version string into a sortable tuple.
#
# @param[in] version Version string.
# @return Tuple of integers for sorting.
##
def parse_version_key(version):
  return tuple(int(part) for part in version.split("."))


##
# @brief Remove a file or directory path when it exists.
#
# @param[in] path Path to remove.
##
def remove_path(path):
  if path.is_dir() and not path.is_symlink():
    shutil.rmtree(path)
  elif path.exists() or path.is_symlink():
    path.unlink()


##
# @brief Replace one directory tree from a source directory.
#
# @param[in] source_dir Source directory path.
# @param[in] destination_dir Destination directory path.
##
def replace_dir(source_dir, destination_dir, ignore_patterns=None):
  if destination_dir.exists():
    shutil.rmtree(destination_dir)

  destination_dir.parent.mkdir(parents=True, exist_ok=True)
  patterns = [".gitkeep"]
  if ignore_patterns is not None:
    patterns.extend(ignore_patterns)

  shutil.copytree(
    source_dir,
    destination_dir,
    ignore=shutil.ignore_patterns(*patterns),
  )


##
# @brief Return whether a directory exists and contains entries other than
# `.gitkeep`.
#
# @param[in] directory Directory path.
# @return True when the directory contains real content.
##
def directory_has_content(directory):
  if not directory.is_dir():
    return False

  return any(path.name != ".gitkeep" for path in directory.iterdir())
