#!/usr/bin/env python3

# ==============================================================================
# @brief Automated embedded target test runner.
#
# Intended for CI and automated hardware testing using CTest.
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
# External imports
# ------------------------------------------------------------------------------
import os
import socket
import subprocess
import sys
import time

# ------------------------------------------------------------------------------
# Project-specific imports
# ------------------------------------------------------------------------------

# ==============================================================================
# CONSTANTS
# ==============================================================================

# Raspberry Pi SSH username.
RPI_USER = None

# Raspberry Pi IP address.
RPI_HOST = None

# TCP port where GDB server will listen.
GDB_PORT = None

# TCP port used for logging.
LOG_PORT = None

# Maximum allowed inactivity time (seconds) without receiving data from target.
TARGET_RX_TIMEOUT_S = None

# Timeout budget (seconds) used for network-latency related waits.
NETWORK_LATENCY_TIMEOUT_S = None

# Absolute path to the workspace folder inside the container.
WORKSPACE_FOLDER = None

# Path to the target ELF firmware to be flashed and executed.
TEST_BINARY = None

# ==============================================================================
# CLASSES
# ==============================================================================

# ==============================================================================
# FUNCTIONS
# ==============================================================================


##
# @brief Retrieve a required environment variable.
#
# Reads the specified environment variable and returns its value.
# If the variable is not defined, prints an error message and
# terminates the program.
#
# @param[in] name Name of the environment variable.
# @return Value of the environment variable as a string.
##
def get_env(name):
  value = os.getenv(name)

  if value is None:
    print(f"❌ Missing required environment variable: {name}", flush=True)
    sys.exit(1)

  return value


##
# @brief Loads required configuration values.
#
# Terminates the program if any required configuration value is missing.
##
def init_config():
  global RPI_USER, RPI_HOST, GDB_PORT, LOG_PORT
  global TARGET_RX_TIMEOUT_S, NETWORK_LATENCY_TIMEOUT_S
  global WORKSPACE_FOLDER, TEST_BINARY

  # Validate command-line arguments
  if len(sys.argv) < 2:
    print("❌ Missing test binary argument.", flush=True)
    sys.exit(1)

  RPI_USER = get_env("RPI_USER")
  RPI_HOST = get_env("RPI_HOST")
  GDB_PORT = int(get_env("GDB_PORT"))
  LOG_PORT = int(get_env("LOG_PORT"))
  TARGET_RX_TIMEOUT_S = float(get_env("TARGET_RX_TIMEOUT_S"))
  NETWORK_LATENCY_TIMEOUT_S = float(get_env("NETWORK_LATENCY_TIMEOUT_S"))
  WORKSPACE_FOLDER = get_env("WORKSPACE_FOLDER")
  TEST_BINARY = sys.argv[1]


##
# @brief Execute a system command.
#
# Runs the given command and optionally suppresses its output.
#
# @param[in] cmd Command to execute as a list.
# @param[in] quiet If True, suppress stdout and stderr.
##
def run(cmd, quiet=True):
  if quiet:
    subprocess.run(
      cmd, check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
    )
  else:
    subprocess.run(cmd, check=True)


##
# @brief Flash firmware to the target via GDB server.
#
# @param[in] test_binary Path to the firmware ELF file.
##
def flash_via_gdb(test_binary):
  print("Flashing target via GDB server...", flush=True)
  # fmt: off
  run([
    "arm-none-eabi-gdb",
    test_binary,
    "-batch",
    "-ex", f"target remote {RPI_HOST}:{GDB_PORT}",
    "-ex", "monitor reset",
    "-ex", "monitor halt",
    "-ex", "load",
    "-ex", "monitor reset",
    "-ex", "monitor halt",
    "-ex", "quit",
  ])
  # fmt: on


##
# @brief Main test execution workflow.
#
# Orchestrates the full test sequence:
# - Start GDB server
# - Flash firmware
# - Start or reuse the configured logging source
# - Connect to logging output
# - Run firmware
# - Capture and analyze output
##
def main():
  init_config()  # Init config values.

  print("Run target GDB server ...", flush=True)
  run([f"{WORKSPACE_FOLDER}/.vscode/tasks/run_target_gdb_server.sh"])

  print("Flashing firmware...", flush=True)
  flash_via_gdb(TEST_BINARY)

  print("Run target logging server ...", flush=True)
  run([f"{WORKSPACE_FOLDER}/.vscode/tasks/run_target_logging_server.sh"])

  print("Connecting to target logging server ...", flush=True)
  logging_socket = socket.create_connection(
    (RPI_HOST, LOG_PORT), timeout=NETWORK_LATENCY_TIMEOUT_S
  )
  logging_socket.settimeout(0.2)

  # Flush stale logging data.
  try:
    while True:
      logging_socket.recv(4096)
  except socket.timeout:
    pass

  # Launch GDB in a subprocess because the "continue" command is blocking.
  # This allows the script to proceed while the target firmware runs.
  print("Starting firmware execution...", flush=True)
  # fmt: off
  gdb_proc = subprocess.Popen(
    [
      "arm-none-eabi-gdb",
      TEST_BINARY,
      "-batch",
      "-ex", f"target remote {RPI_HOST}:{GDB_PORT}",
      "-ex", "monitor reset",
      "-ex", "continue",
    ],
    stdout=subprocess.DEVNULL,
    stderr=subprocess.DEVNULL,
  )
  # fmt: on

  print("Capturing test output:", flush=True)
  buffer = ""
  last_rx = time.time()  # Timestamp of last received data.

  ret = 1  # Set default exit code to failure.
  try:
    while True:
      try:
        data = logging_socket.recv(4096).decode(errors="replace")
      except socket.timeout:
        data = ""

      if data:
        # Update timestamp when data is received.
        last_rx = time.time()

        print(data, end="", flush=True)
        buffer += data

        if "0 failed" in buffer:
          print("\n✅ Test pass.", flush=True)
          ret = 0
          break

        if "FAIL" in buffer:
          print("\n❌ Test fail.", flush=True)
          break

      # Exit if no data has been received for TARGET_RX_TIMEOUT_S.
      if (time.time() - last_rx) > TARGET_RX_TIMEOUT_S:
        print("\n❌ Timeout: no output from target.", flush=True)
        break

  finally:
    # Cleanup.
    logging_socket.close()
    gdb_proc.kill()
    gdb_proc.wait()

    # Exit with appropriate exit code.
    sys.exit(ret)


# ==============================================================================
# SCRIPT ENTRY POINT
# ==============================================================================

if __name__ == "__main__":
  main()
