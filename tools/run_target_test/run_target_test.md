# Run target test overview

`run_target_test.py` automates remote Hardware-in-the-Loop target execution by flashing a test binary through GDB, enabling remote logging, running the firmware and translating the observed result into a process exit code suitable for `CTest`.

It works as follows:
- Starting GDB server on a remote Raspberry Pi.
- Flashing firmware via the GDB server.
- Starting a serial-to-TCP bridge.
- Running the target firmware.
- Capturing serial output.
- Reporting PASS/FAIL.

# Glossary

| Term | Definition |
|---|---|
|   |   |

# Usage example

Arguments:
- `<test_binary>`: Absolute path to the target ELF firmware to be executed.

Environment:
- `RPI_USER`: Raspberry Pi SSH username.
- `RPI_HOST`: Raspberry Pi IP address.
- `GDB_PORT`: TCP port where GDB server will listen.
- `LOG_PORT`: TCP port used for logging.
- `TARGET_RX_TIMEOUT_S`: Maximum allowed inactivity time (seconds) without receiving data from the target.
- `NETWORK_LATENCY_TIMEOUT_S`: Timeout budget (seconds) used for network-latency waits.
- `WORKSPACE_FOLDER`: Absolute path to the workspace folder inside the container.

Run one target test binary manually after exporting the required environment variables:

```bash
python tools/run_target_test/run_target_test.py build/target/example_test
```
