#!/usr/bin/env python3

# ==============================================================================
# @brief Restore the KiCad state into the persistent runtime when needed.
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
import sys

# ------------------------------------------------------------------------------
# External library imports
# ------------------------------------------------------------------------------
# ------------------------------------------------------------------------------
# Project-specific imports
# ------------------------------------------------------------------------------
from kicad_state_common import directory_has_content
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
# @brief Detect the active KiCad major.minor version.
#
# @return Version string such as `10.0`, or None when detection fails.
##
def detect_kicad_version():
  versions = get_detected_versions()
  if versions:
    return versions[-1]

  state_root_dir = get_state_root_dir()
  if state_root_dir.exists():
    state_versions = sorted(
      [path.name for path in state_root_dir.iterdir() if path.is_dir()]
    )
    if len(state_versions) == 1:
      return state_versions[0]

  return None


##
# @brief Return whether the persistent runtime already contains KiCad state.
#
# @param[in] target_config_dir Runtime config directory.
# @param[in] target_documents_dir Runtime documents directory.
# @return True when either directory already contains real content.
##
def runtime_has_content(target_config_dir, target_documents_dir):
  return directory_has_content(target_config_dir) or directory_has_content(
    target_documents_dir
  )


##
# @brief Restore one optional state directory into the runtime.
#
# @param[in] source_dir Directory in the state.
# @param[in] target_dir Destination directory in the runtime.
##
def restore_dir(source_dir, target_dir):
  if source_dir.is_dir():
    replace_dir(source_dir, target_dir)
  else:
    remove_path(target_dir)


##
# @brief Parse command-line arguments.
#
# @return Parsed arguments namespace.
##
def parse_args():
  parser = argparse.ArgumentParser()
  parser.add_argument(
    "--force",
    action="store_true",
    help="replace the current KiCad runtime with the stored state",
  )
  return parser.parse_args()


##
# @brief Restore the KiCad state when the runtime is empty or force is used.
##
def main():
  args = parse_args()
  kicad_version = detect_kicad_version()
  if kicad_version is None:
    print("KiCad version could not be detected. Skipping KiCad state restore.")
    return 0

  state_dir = get_state_root_dir() / kicad_version
  if not state_dir.is_dir():
    print(
      f"No KiCad state stored for version {kicad_version}. "
      "Skipping KiCad state restore."
    )
    return 0

  target_config_dir = get_config_root() / kicad_version
  target_documents_dir = get_versioned_documents_dir(kicad_version)
  state_config_dir = state_dir / "config"
  state_documents_dir = state_dir / "documents"

  if not args.force and runtime_has_content(
    target_config_dir, target_documents_dir
  ):
    print(f"KiCad runtime already initialized for version {kicad_version}.")
    return 0

  restore_dir(state_config_dir, target_config_dir)
  restore_dir(state_documents_dir, target_documents_dir)

  print(f"KiCad state restored for version {kicad_version}.")
  return 0


# ==============================================================================
# SCRIPT ENTRY POINT
# ==============================================================================

if __name__ == "__main__":
  sys.exit(main())
