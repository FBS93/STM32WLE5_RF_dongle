#!/usr/bin/env python3

# ==============================================================================
# @brief Forward J-Link SWO Viewer output to one TCP client.
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
import select
import shutil
import signal
import socket
import subprocess
import sys
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

## @brief Server identity used by run_logging_server.sh.
SERVER_TAG = "stm32wle5c8u6-jlink_swo-v1"
## @brief Fixed Viewer arguments preceding the output file option.
VIEWER_ARGUMENTS = (
  "-device STM32WLE5C8 -swoattach 1 -swofreq 2000000 -itmport 0".split()
)
## @brief Maximum capture file size before Viewer restart.
CAPTURE_LIMIT_BYTES = 16 * 1024 * 1024
## @brief Maximum bytes waiting for the TCP client.
PENDING_LIMIT_BYTES = 256 * 1024
## @brief Socket polling interval in seconds.
POLL_SECONDS = 0.05
## @brief File read size in bytes.
READ_BYTES = 65536

# ==============================================================================
# GLOBAL VARIABLES
# ==============================================================================

## @brief Whether process shutdown was requested.
stop_requested = False

# ==============================================================================
# CLASSES
# ==============================================================================

# ==============================================================================
# FUNCTIONS
# ==============================================================================


##
# @brief Request shutdown from a signal handler.
# @param[in] _signum Signal number supplied by the signal module.
# @param[in] _frame Current stack frame supplied by the signal module.
##
def request_stop(_signum, _frame):
  global stop_requested

  stop_requested = True


##
# @brief Stop and reap Viewer, escalating when it does not terminate.
#
# @param[in] process Viewer process to stop, or @c None.
##
def stop_viewer(process):
  if process is None:
    return
  if process.poll() is None:
    process.terminate()
    try:
      process.wait(timeout=2)
    except subprocess.TimeoutExpired:
      process.kill()
  process.wait()


##
# @brief Restart Viewer on a fresh capture file and return unread bytes.
#
# @param[in] viewer Path to the Viewer executable.
# @param[in] output_path Viewer capture file path.
# @param[in] process Existing Viewer process, or @c None.
# @param[in] output Existing capture file object, or @c None.
# @return New Viewer process, capture file object, and unread trailing bytes.
##
def restart_capture(viewer, output_path, process, output):
  stop_viewer(process)
  trailing_data = b""
  if output is not None:
    trailing_data = output.read()
    output.close()

  # Stop Viewer before replacement so its write offset cannot create a hole.
  output_path.unlink(missing_ok=True)
  output_path.touch(mode=0o600)
  process = subprocess.Popen(
    [viewer, *VIEWER_ARGUMENTS, "-outputfile", str(output_path)],
    stdin=subprocess.DEVNULL,
    stdout=subprocess.DEVNULL,
    stderr=subprocess.DEVNULL,
  )
  return process, output_path.open("rb", buffering=0), trailing_data


##
# @brief Run the single-client SWO TCP server.
#
# @return Process exit code.
##
def main():
  if len(sys.argv) != 3:
    print("❌ Error: expected LOG_PORT and server tag arguments.", flush=True)
    return 1
  try:
    log_port = int(sys.argv[1])
  except ValueError:
    print("❌ Error: LOG_PORT must be an integer.", flush=True)
    return 1
  if not 1 <= log_port <= 65535 or sys.argv[2] != SERVER_TAG:
    print("❌ Error: invalid LOG_PORT or server tag.", flush=True)
    return 1

  viewer = shutil.which("JLinkSWOViewerCLExe")
  if viewer is None:
    print("❌ Error: JLinkSWOViewerCLExe not found.", flush=True)
    return 1

  output_path = Path(f"/tmp/stm32wle5c8u6_jlink_swo_capture_{log_port}.log")
  server = client = process = output = None
  capture_owned = False
  pending = bytearray()

  # Request a clean shutdown when the remote server is stopped.
  signal.signal(signal.SIGTERM, request_stop)
  signal.signal(signal.SIGINT, request_stop)

  try:
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("0.0.0.0", log_port))
    server.listen()
    server.setblocking(False)

    capture_owned = True
    process, output, _ = restart_capture(viewer, output_path, None, None)
    try:
      process.wait(timeout=0.2)
      raise RuntimeError(f"Viewer exited with status {process.returncode}")
    except subprocess.TimeoutExpired:
      pass
    if not stop_requested:
      print(
        f"✅ STM32WLE5C8U6 J-Link SWO TCP server ready on port {log_port}.",
        flush=True,
      )

    while not stop_requested:
      if process.poll() is not None:
        raise RuntimeError(f"Viewer exited with status {process.returncode}")

      # Monitor client input to detect disconnection while capture is idle.
      readers = [server] + ([client] if client is not None else [])
      writers = [client] if client is not None and pending else []
      readable, writable, _ = select.select(readers, writers, [], POLL_SECONDS)

      if server in readable:
        new_client, _ = server.accept()
        new_client.setblocking(False)
        if client is not None:
          client.close()
        client = new_client
        pending.clear()

      if client is not None and client in readable:
        try:
          peer_data = client.recv(READ_BYTES)
        except OSError:
          peer_data = b""
        if not peer_data:
          client.close()
          client = None
          pending.clear()

      if client is not None and client in writable:
        try:
          sent = client.send(pending)
          if sent == 0:
            raise ConnectionError
          del pending[:sent]
        except OSError:
          client.close()
          client = None
          pending.clear()

      # Read only the Viewer output appended since the previous iteration.
      captured = output.read(READ_BYTES)

      # Restart Viewer before replacing a capture file that reached its limit.
      if os.fstat(output.fileno()).st_size >= CAPTURE_LIMIT_BYTES:
        process, output, trailing = restart_capture(
          viewer, output_path, process, output
        )
        captured += trailing

      if captured and client is not None:
        if len(pending) + len(captured) > PENDING_LIMIT_BYTES:
          # Disconnect rather than allowing one slow client to stop capture.
          client.close()
          client = None
          pending.clear()
        else:
          pending.extend(captured)
  except (OSError, RuntimeError) as error:
    print(f"❌ Error: {error}.", file=sys.stderr, flush=True)
    return 1
  finally:
    if client is not None:
      client.close()
    if server is not None:
      server.close()
    stop_viewer(process)
    if output is not None:
      output.close()
    if capture_owned:
      output_path.unlink(missing_ok=True)

  return 0


# ==============================================================================
# SCRIPT ENTRY POINT
# ==============================================================================

if __name__ == "__main__":
  sys.exit(main())
