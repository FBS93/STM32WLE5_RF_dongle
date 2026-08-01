# ==============================================================================
# @brief Minimal remote Sigrok wrapper for Raspberry Pi based workflows.
#
# This module forwards Sigrok CLI invocations to the Raspberry Pi gateway
# without redefining or validating Sigrok-specific arguments. SCP transport is
# kept as an internal implementation detail so automation uses the wrapper API
# instead of raw transfer primitives.
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
import posixpath
import shlex
import subprocess
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

# ==============================================================================
# CLASSES
# ==============================================================================


##
# @brief Minimal remote client for Sigrok-based logic analyzer automation.
##
class SigrokRemoteClient:
  ##
  # @brief Initialize a remote Sigrok client.
  #
  # If any argument is omitted, the corresponding value is loaded from the
  # process environment.
  #
  # @param[in] rpi_user Raspberry Pi SSH username.
  # @param[in] rpi_host Raspberry Pi SSH host.
  # @param[in] sigrok_device Sigrok device selector forwarded through
  # `--driver`.
  # @param[in] artifacts_dir Local root directory used to store fetched
  # artifacts.
  ##
  def __init__(
    self,
    rpi_user=None,
    rpi_host=None,
    sigrok_device=None,
    artifacts_dir=None,
  ):
    rpi_user = os.getenv("RPI_USER") if rpi_user is None else rpi_user
    rpi_host = os.getenv("RPI_HOST") if rpi_host is None else rpi_host
    sigrok_device = (
      os.getenv("LOGIC_ANALYZER_DEVICE")
      if sigrok_device is None
      else sigrok_device
    )
    artifacts_dir = (
      os.getenv("LOGIC_ANALYZER_ARTIFACTS_DIR")
      if artifacts_dir is None
      else artifacts_dir
    )

    if rpi_user is None:
      raise RuntimeError("Missing required environment variable: RPI_USER")

    if rpi_host is None:
      raise RuntimeError("Missing required environment variable: RPI_HOST")

    if sigrok_device is None:
      raise RuntimeError(
        "Missing required environment variable: LOGIC_ANALYZER_DEVICE"
      )

    if artifacts_dir is None:
      raise RuntimeError(
        "Missing required environment variable: LOGIC_ANALYZER_ARTIFACTS_DIR"
      )

    self.rpi_user = rpi_user
    self.rpi_host = rpi_host
    self.sigrok_device = sigrok_device
    self.artifacts_dir = Path(artifacts_dir)

  ##
  # @brief Run `sigrok-cli` synchronously on the Raspberry Pi.
  #
  # @param[in] sigrok_args Positional arguments forwarded to `sigrok-cli`.
  # @return Completed subprocess result.
  ##
  def run(self, *sigrok_args):
    return self._run_remote_command(*self._sigrok_command(*sigrok_args))

  ##
  # @brief Run `sigrok-cli` asynchronously on the Raspberry Pi.
  #
  # @param[in] sigrok_args Positional arguments forwarded to `sigrok-cli`.
  # @return Running subprocess handle.
  ##
  def run_async(self, *sigrok_args):
    return self._run_remote_command_async(*self._sigrok_command(*sigrok_args))

  ##
  # @brief Run `sigrok-cli` remotely without forcing the configured device.
  #
  # Intended for offline processing of previously captured artifacts such as
  # format conversion from `.sr` into other export formats.
  #
  # @param[in] sigrok_args Positional arguments forwarded to `sigrok-cli`.
  # @return Completed subprocess result.
  ##
  def run_offline(self, *sigrok_args):
    return self._run_remote_command("sigrok-cli", *sigrok_args)

  ##
  # @brief Fetch one remote capture artifact into the configured local
  # artifacts root.
  #
  # Recommended usage is to pass `capture_name` equal to the test
  # case name so the resulting artifact tree is grouped as
  # `<LOGIC_ANALYZER_ARTIFACTS_DIR>/<capture_name>/...`.
  #
  # @param[in] capture_name Logical capture group name, typically the
  # test case name.
  # @param[in] remote_path Remote source path on the Raspberry Pi.
  # @return Local artifact path.
  ##
  def fetch_artifact(self, capture_name, remote_path):
    capture_dir = self._capture_artifacts_dir(capture_name)
    local_name = posixpath.basename(str(remote_path))
    local_path = capture_dir / self._validate_artifact_file_name(local_name)
    self._copy_from(remote_path, local_path)

    return local_path

  ##
  # @brief Copy a file from the Raspberry Pi to the local workspace.
  #
  # @param[in] remote_path Remote source path on the Raspberry Pi.
  # @param[in] local_path Local destination path.
  #
  # Internal transport primitive used by the public artifact-fetching helpers.
  # Direct SCP transfers are intentionally not part of the recommended
  # public API.
  ##
  def _copy_from(self, remote_path, local_path):
    local_path = Path(local_path)
    local_path.parent.mkdir(parents=True, exist_ok=True)

    subprocess.run(
      [
        "scp",
        "-o",
        "StrictHostKeyChecking=accept-new",
        self._scp_remote_path(remote_path),
        os.fspath(local_path),
      ],
      check=True,
    )

  ##
  # @brief Build one SSH command line.
  #
  # @param[in] command_args Remote command and arguments.
  # @return SSH command line ready for subprocess execution.
  ##
  def _ssh_command(self, *command_args):
    return [
      "ssh",
      "-o",
      "StrictHostKeyChecking=accept-new",
      self._ssh_target(),
      *[str(command_arg) for command_arg in command_args],
    ]

  ##
  # @brief Run one remote command through a shell-safe SSH invocation.
  #
  # @param[in] command_args Remote command and arguments.
  # @return Completed subprocess result.
  ##
  def _run_remote_command(self, *command_args):
    remote_command = shlex.join(
      [str(command_arg) for command_arg in command_args]
    )

    return subprocess.run(
      self._ssh_command("sh", "-lc", shlex.quote(remote_command)),
      check=True,
    )

  ##
  # @brief Start one remote command through SSH and return immediately.
  #
  # @param[in] command_args Remote command and arguments.
  # @return Running subprocess handle.
  ##
  def _run_remote_command_async(self, *command_args):
    remote_command = shlex.join(
      [str(command_arg) for command_arg in command_args]
    )

    return subprocess.Popen(
      self._ssh_command("sh", "-lc", shlex.quote(remote_command))
    )

  ##
  # @brief Build the SSH target string.
  #
  # @return SSH target in `user@host` format.
  ##
  def _ssh_target(self):
    return f"{self.rpi_user}@{self.rpi_host}"

  ##
  # @brief Build one SCP remote path specification.
  #
  # @param[in] remote_path Remote path on the Raspberry Pi.
  # @return SCP remote path specification.
  ##
  def _scp_remote_path(self, remote_path):
    return f"{self._ssh_target()}:{shlex.quote(str(remote_path))}"

  ##
  # @brief Resolve one local capture artifact directory.
  #
  # @param[in] capture_name Logical capture group name.
  # @return Capture artifact directory path.
  ##
  def _capture_artifacts_dir(self, capture_name):
    capture_relative_path = self._validate_capture_name(capture_name)
    capture_dir = self.artifacts_dir / capture_relative_path
    capture_dir.mkdir(parents=True, exist_ok=True)

    return capture_dir

  ##
  # @brief Build one `sigrok-cli` command line with the configured device.
  #
  # @param[in] sigrok_args Positional arguments forwarded to `sigrok-cli`.
  # @return Sigrok CLI command line.
  ##
  def _sigrok_command(self, *sigrok_args):
    if "--driver" in sigrok_args or "-d" in sigrok_args:
      raise ValueError(
        "Sigrok device selection must be configured through "
        "LOGIC_ANALYZER_DEVICE"
      )

    return ["sigrok-cli", "--driver", self.sigrok_device, *sigrok_args]

  ##
  # @brief Validate a logical capture name used below the artifacts root.
  #
  # @param[in] capture_name Logical capture group name.
  # @return Relative capture path.
  ##
  def _validate_capture_name(self, capture_name):
    capture_path = Path(capture_name)

    if capture_path.is_absolute():
      raise ValueError(
        "capture_name must be relative to LOGIC_ANALYZER_ARTIFACTS_DIR"
      )

    if capture_path == Path("."):
      raise ValueError("capture_name must not be empty")

    if ".." in capture_path.parts:
      raise ValueError(
        "capture_name must not escape LOGIC_ANALYZER_ARTIFACTS_DIR"
      )

    return capture_path

  ##
  # @brief Validate one local artifact file name.
  #
  # @param[in] local_name Local artifact file name.
  # @return File name safe to append below the capture directory.
  ##
  def _validate_artifact_file_name(self, local_name):
    local_name_path = Path(local_name)
    has_single_part = len(local_name_path.parts) == 1
    matches_file_name = local_name_path.name == str(local_name_path)

    if local_name_path.is_absolute():
      raise ValueError("local_name must be relative to the capture directory")

    if local_name_path == Path("."):
      raise ValueError("local_name must not be empty")

    if not has_single_part or not matches_file_name:
      raise ValueError("local_name must be a file name, not a path")

    return local_name_path


# ==============================================================================
# FUNCTIONS
# ==============================================================================
