# Embedded target remote logging

## Overview

This document describes two modes for remote embedded target logging using a Raspberry Pi:
- Serial mode provides a managed backend for serial devices.
- TCP mode allows any external logging source to be used as long as it exposes a textual TCP endpoint on the configured logging port.

Both modes expose logging to the host through the same configured TCP port.

## Architecture

### Serial mode

- Embedded target outputs logs via a serial peripheral.
- A USB-to-serial adapter connects the target to the Raspberry Pi.
- EW runs the `run_target_serial_logging_server` tool on the Raspberry Pi and exposes `LOG_PORT`.
- The host or HIL workflow connects to `LOG_PORT` to receive logs.

Target serial interface --> USB-Serial Adapter --> Raspberry Pi --> TCP --> Host

### TCP mode

- A provider-specific TCP logging source provides the target logs.
- The configured command for the TCP provider prepares or reuses that logging source and exposes `LOG_PORT`.
- The host or HIL workflow connects to `LOG_PORT` to receive logs.

Provider-specific TCP logging source --> TCP --> Host

## Logging Setup

Logging is enabled by executing [run_target_logging_server.sh](../../../../.vscode/tasks/run_target_logging_server.sh).

### Serial mode

For `LOG_SOURCE=serial`, this script:
- Copies the [run_target_serial_logging_server tool](../../../../tools/run_target_serial_logging_server/run_target_serial_logging_server.md) to the Raspberry Pi.
- Connects to the Raspberry Pi via SSH.
- Uses the configured serial device.
- Reuses an already healthy running target logging server when available.
- Otherwise, starts the copied script on the Raspberry Pi with the selected serial device, TCP port and baud rate.
- Verifies that the target logging server is running.

### TCP mode

For `LOG_SOURCE=tcp`, this script:
- Executes `LOG_TCP_RUN_CMD`.
- Verifies that `LOG_PORT` is available.

## Dependencies

### Raspberry Pi

For serial mode, the Raspberry Pi shall have Python 3 installed (default in Raspberry Pi OS).

### Devcontainer

Make sure that the environment variables in [devcontainer.json](../../../../.devcontainer/devcontainer.json) are configured for the logging setup and target environment:

- `RPI_USER` and `RPI_HOST` shall match the Raspberry Pi SSH credentials.
- `LOG_SOURCE` is mandatory and shall be `serial` or `tcp`.
- `LOG_PORT` shall match the port used for logging.
- `LOG_BAUD_RATE` is used only for `LOG_SOURCE=serial` and shall match the target serial configuration.
- `LOG_SERIAL_DEVICE` is used only for `LOG_SOURCE=serial` and shall match the Raspberry Pi serial device path assigned to target logging.
- `LOG_TCP_RUN_CMD` is used only for `LOG_SOURCE=tcp`. It defines the command used to set up the TCP logging provider. Repeated executions of this command shall be idempotent.
- `NETWORK_LATENCY_TIMEOUT_S` defines the maximum wait time used by readiness checks for the logging server.
- `WORKSPACE_FOLDER` shall be the absolute workspace path.

To discover `LOG_SERIAL_DEVICE` on the Raspberry Pi, inspect the USB serial symlinks:

```bash
ls -l /dev/serial/by-id/
```

Copy the full `/dev/serial/by-id/...` path into `LOG_SERIAL_DEVICE` and reopen or rebuild the devcontainer so the updated environment variable is reloaded.
