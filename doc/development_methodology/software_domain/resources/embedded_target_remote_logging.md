# Embedded target remote logging

## Overview

This document describes how embedded target logging is performed remotely using a Raspberry Pi acting as a serial-to-TCP bridge.
A USB-to-serial adapter is connected between the target device and the Raspberry Pi, allowing serial peripheral output to be forwarded over the network.

## Architecture

- Embedded target outputs logs via serial peripheral.
- A USB-to-serial adapter connects the target to the Raspberry Pi.
- The Raspberry Pi runs a serial-to-TCP bridge.
- The host machine connects to the Raspberry Pi TCP port to receive logs.

Target serial interface --> USB-Serial Adapter --> Raspberry Pi --> TCP --> Host

## Logging Setup

Logging is enabled by executing [run_target_logging_server.sh](../../../../.vscode/tasks/run_target_logging_server.sh)

This script:
- Copies the script provided by the [run_target_logging_server tool](../../../../tools/run_target_logging_server/run_target_logging_server.md) to the Raspberry Pi.
- Connects to the Raspberry Pi via SSH.
- Uses the configured serial device.
- Reuses an already healthy running target logging server when available.
- If not, starts the script provided by the [run_target_logging_server tool](../../../../tools/run_target_logging_server/run_target_logging_server.md) on the Raspberry Pi with the selected serial device, configured TCP port and configured baud rate.
- Verifies that the target logging server is running.

The host can then connect to the configured TCP port to receive runtime logs.

## Dependencies

### Raspberry Pi

The Raspberry Pi shall have Python 3 installed (default in Raspberry Pi OS).

### Devcontainer

Make sure that the environment variables in [devcontainer.json](../../../../.devcontainer/devcontainer.json) are configured for the logging setup and target environment:

- `RPI_USER` and `RPI_HOST` shall match the Raspberry Pi SSH credentials.
- `LOG_PORT` shall match the port used for serial logging.
- `LOG_BAUD_RATE` shall match the target serial configuration.
- `LOG_SERIAL_DEVICE` shall match the Raspberry Pi serial device path assigned to target logging.
- `NETWORK_LATENCY_TIMEOUT_S` defines the maximum wait time used by readiness checks for the logging server.

To discover `LOG_SERIAL_DEVICE` on the Raspberry Pi, inspect the USB serial symlinks:

```bash
ls -l /dev/serial/by-id/
```

Copy the full `/dev/serial/by-id/...` path into `LOG_SERIAL_DEVICE` and reopen or rebuild the devcontainer so the updated environment variable is reloaded.
