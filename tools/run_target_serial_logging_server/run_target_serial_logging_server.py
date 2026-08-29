#!/usr/bin/env python3

# ==============================================================================
# @brief TCP logging bridge for Raspberry Pi.
#
# Intended to be launched remotely from run_target_logging_server.sh.
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
import select
import socket
import sys
import termios
import tty

# ------------------------------------------------------------------------------
# Project-specific imports
# ------------------------------------------------------------------------------

# ==============================================================================
# CONSTANTS
# ==============================================================================

# Maximum concurrent TCP clients (0 means no explicit limit).
MAX_CLIENTS = 0

# ==============================================================================
# CLASSES
# ==============================================================================

# ==============================================================================
# FUNCTIONS
# ==============================================================================


##
# @brief Map integer baud rate to termios constant.
#
# @param[in] baud Requested serial baud rate.
# @return termios baud constant.
##
def baud_to_termios(baud):
  mapping = {
    9600: termios.B9600,
    19200: termios.B19200,
    38400: termios.B38400,
    57600: termios.B57600,
    115200: termios.B115200,
    230400: termios.B230400,
  }

  if baud not in mapping:
    print(f"❌ Unsupported LOG_BAUD_RATE: {baud}", flush=True)
    sys.exit(1)

  return mapping[baud]


##
# @brief Open and configure serial device in raw mode.
#
# @param[in] serial_dev Path to serial device.
# @param[in] baud_rate Baud rate to configure.
# @return Open file descriptor to serial device.
##
def open_serial_raw(serial_dev, baud_rate):
  try:
    serial_fd = os.open(serial_dev, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
  except OSError as e:
    print(f"❌ Failed to open serial device {serial_dev}: {e}", flush=True)
    sys.exit(1)

  attrs = termios.tcgetattr(serial_fd)
  baud = baud_to_termios(baud_rate)
  tty.setraw(serial_fd)
  attrs = termios.tcgetattr(serial_fd)
  attrs[4] = baud  # ispeed
  attrs[5] = baud  # ospeed
  termios.tcsetattr(serial_fd, termios.TCSANOW, attrs)

  return serial_fd


##
# @brief Main TCP logging bridge workflow.
#
# Orchestrates the full bridge sequence:
# - Validate input arguments.
# - Open and configure serial interface.
# - Start TCP logging server.
# - Accept TCP client connections.
# - Broadcast target serial RX to all clients.
# - Forward client TCP TX to target serial.
##
def main():
  if len(sys.argv) != 4:
    print(
      "Usage: run_target_serial_logging_server.py",
      "<serial_device>",
      "<tcp_port>",
      "<baud_rate>",
      flush=True,
    )
    sys.exit(1)

  serial_dev = sys.argv[1]
  try:
    tcp_port = int(sys.argv[2])
  except ValueError:
    print(f"❌ Invalid TCP port: {sys.argv[2]}", flush=True)
    sys.exit(1)

  try:
    baud_rate = int(sys.argv[3])
  except ValueError:
    print(f"❌ Invalid baud rate: {sys.argv[3]}", flush=True)
    sys.exit(1)

  serial_fd = open_serial_raw(serial_dev, baud_rate)

  server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
  server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
  try:
    server.bind(("0.0.0.0", tcp_port))
    server.listen()
    server.setblocking(False)
  except OSError as e:
    os.close(serial_fd)
    print(f"❌ Failed to bind/listen on TCP port {tcp_port}: {e}", flush=True)
    sys.exit(1)

  print(
    f"✅ Python logging bridge ready on TCP port {tcp_port} for {serial_dev}",
    flush=True,
  )

  clients = set()
  while True:
    monitored = [server, serial_fd] + list(clients)
    readable, _, _ = select.select(monitored, [], [], 1.0)

    # Accept all pending incoming TCP connections.
    if server in readable:
      while True:
        try:
          client, _ = server.accept()
        except BlockingIOError:
          break
        except OSError:
          break

        client.setblocking(False)
        if MAX_CLIENTS > 0 and len(clients) >= MAX_CLIENTS:
          try:
            client.close()
          except OSError:
            pass
          continue

        clients.add(client)

    # Broadcast target serial RX to every connected TCP client.
    if serial_fd in readable:
      try:
        data = os.read(serial_fd, 4096)
      except OSError:
        data = b""

      if data:
        disconnected = []
        for client in clients:
          try:
            client.sendall(data)
          except (BrokenPipeError, ConnectionResetError, OSError):
            disconnected.append(client)

        for client in disconnected:
          clients.discard(client)
          try:
            client.close()
          except OSError:
            pass

    # Forward TCP TX from any client to the target serial interface.
    disconnected = []
    for client in list(clients):
      if client not in readable:
        continue

      try:
        cmd = client.recv(1024)
      except (ConnectionResetError, OSError):
        # Client socket broke unexpectedly.
        disconnected.append(client)
        continue

      if cmd == b"":
        # EOF: peer closed the TCP connection.
        disconnected.append(client)
        continue

      if cmd:
        try:
          os.write(serial_fd, cmd)
        except OSError:
          pass

    for client in disconnected:
      # Remove and close all sockets marked as disconnected.
      clients.discard(client)
      try:
        client.close()
      except OSError:
        pass


# ==============================================================================
# SCRIPT ENTRY POINT
# ==============================================================================

if __name__ == "__main__":
  main()
