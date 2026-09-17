# STM32WLE5C8U6 J-Link SWO TCP server overview

`run_logging_server.sh` and `swo_tcp_server.py` provide the project-specific TCP logging adapter for J-Link SWO output. The shell adapter prepares the remote service, while the Python server manages SWO Viewer output and exposes it to one TCP client.

The combined tool works as follows:
1. The generic [EW](../../../doc/ew.md) GDB server runner starts JLinkGDBServer, which acquires the J-Link probe first.
2. The shell adapter deploys the Python server to the Raspberry Pi or reuses an exact, healthy server already listening on `LOG_PORT`.
3. The Python server starts J-Link SWO Viewer in attach mode. SWO Viewer attaches to the J-Link probe already acquired by JLinkGDBServer.
4. Target firmware initializes ITM, TPIU, and SWO. SWO Viewer writes the output to `/tmp/stm32wle5c8u6_jlink_swo_capture_<LOG_PORT>.log`, and the Python server exposes newly written data on `LOG_PORT`.
5. GDB clients and logging TCP clients connect and disconnect independently while JLinkGDBServer retains probe ownership.

File roles:
- `run_logging_server.sh` starts the generic [EW](../../../doc/ew.md) GDB server runner, deploys the provider, and applies its reuse and stale-listener policy.
- `swo_tcp_server.py` runs SWO Viewer, manages its capture file, and serves the captured output on `LOG_PORT`.

For the generic [EW](../../../doc/ew.md) logging and debugging workflows, see [embedded target remote logging](../../../doc/development_methodology/software_domain/resources/embedded_target_remote_logging.md) and [embedded target remote debugging](../../../doc/development_methodology/software_domain/resources/embedded_target_remote_debugging.md).

# Glossary

| Term | Definition |
|---|---|
| ITM | Instrumentation Trace Macrocell used by firmware to emit trace data. |
| SWO | Serial Wire Output trace signal transported through the J-Link probe. |
| SWO Viewer | SEGGER tool that attaches to the acquired J-Link probe and records SWO output. |
| Logging TCP client | Host-side consumer that connects to the Python server on `LOG_PORT`. |

# Usage example

1. Configure `LOG_SOURCE=tcp` and set `LOG_TCP_RUN_CMD` to `tools/target_adapters/stm32wle5c8u6_jlink_swo/run_logging_server.sh`. The adapter uses the existing `RPI_USER`, `RPI_HOST`, `LOG_PORT` and `WORKSPACE_FOLDER` environment variables. The generic GDB runner also uses the configured `GDB_PORT`, `GDB_SERVER_RUN_CMD`, and `NETWORK_LATENCY_TIMEOUT_S` environment variables.
2. Provide `ssh` and `scp` in the development container. On the Raspberry Pi provide Python 3 and the SEGGER J-Link tools including `JLinkGDBServer` and `JLinkSWOViewerCLExe`.
3. Start logging through the generic EW logging task, which invokes `LOG_TCP_RUN_CMD`; do not invoke the SWO TCP server implementation directly. An exact healthy SWO TCP server is reused. Otherwise, listeners on the dedicated `LOG_PORT` and an identifiable stale SWO TCP servers are stopped before replacement. Stopping the SWO TCP server terminates SWO Viewer and removes its temporary capture file; the GDB server lifecycle remains separate.

Conditions and limitations:
- Use a dedicated Raspberry Pi and `LOG_PORT` for this adapter.
- JLinkGDBServer must remain the probe owner.
- Logging after a target reset depends on firmware reinitializing ITM, TPIU, and SWO.
