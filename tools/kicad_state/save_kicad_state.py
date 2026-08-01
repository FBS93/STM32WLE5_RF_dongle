#!/usr/bin/env python3

# ==============================================================================
# @brief Capture the current KiCad runtime into the repository state.
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
import sys

# ------------------------------------------------------------------------------
# External library imports
# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------
# Project-specific imports
# ------------------------------------------------------------------------------
from kicad_state_common import get_config_root
from kicad_state_common import get_detected_versions
from kicad_state_common import get_state_root_dir
from kicad_state_common import get_versioned_documents_dir
from kicad_state_common import remove_path
from kicad_state_common import replace_dir

# ==============================================================================
# CONSTANTS
# ==============================================================================

# ==============================================================================
# CLASSES
# ==============================================================================

# ==============================================================================
# FUNCTIONS
# ==============================================================================


##
# @brief Detect the active KiCad version from the runtime config directory.
#
# @return Version string such as `10.0`, or None when detection fails.
##
def detect_kicad_version():
  versions = get_detected_versions()
  if versions:
    return versions[-1]

  return None


##
# @brief Capture one runtime directory into the versioned state.
#
# @param[in] source_dir Runtime directory.
# @param[in] state_dir Destination directory in the repository.
##
def capture_dir(source_dir, state_dir, ignore_patterns=None):
  if source_dir.is_dir():
    replace_dir(source_dir, state_dir, ignore_patterns)
  else:
    remove_path(state_dir)


##
# @brief Capture the current KiCad runtime into the repository state.
##
def main():
  kicad_version = detect_kicad_version()
  if kicad_version is None:
    print(f"KiCad version could not be detected in {get_config_root()}.")
    return 1

  source_config_dir = get_config_root() / kicad_version
  source_documents_dir = get_versioned_documents_dir(kicad_version)
  state_dir = get_state_root_dir() / kicad_version

  if not source_config_dir.is_dir() and not source_documents_dir.is_dir():
    print("KiCad runtime is empty. Nothing to save.")
    return 1

  capture_dir(source_config_dir, state_dir / "config")
  capture_dir(source_documents_dir, state_dir / "documents", ["3rdparty"])

  print(f"KiCad state saved for version {kicad_version}.")
  return 0


# ==============================================================================
# SCRIPT ENTRY POINT
# ==============================================================================

if __name__ == "__main__":
  sys.exit(main())
