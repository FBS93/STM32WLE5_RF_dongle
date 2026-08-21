# CLI requirements

# SYS_REQ_CLI_1

The system shall provide the executable `stm32wle5-rf-dongle` for manual and scripted operation on a POSIX-compatible host, invoked as `stm32wle5-rf-dongle --device <path> <command> [options]`, where `--device <path>` selects the product Virtual COM Port.

Validation criteria:
- Invoke each supported command interactively and from a non-interactive script and verify access through the explicitly selected device.
- Verify that invocation without `--device <path>` is rejected before product access.

Domain:
- sw

Upstream traceability:
- [SPEC_1](../../spec/specs.md#spec_1)
- [SPEC_2](../../spec/specs.md#spec_2)
- [SPEC_3](../../spec/specs.md#spec_3)

References:
- [SYS_REQ_INTERFACE_1](sys_interfaces.md#sys_req_interface_1)

Status: approved

# SYS_REQ_CLI_2

The system shall expose only the following CLI commands and map them to [sysi_hostUsbControl](sys_interfaces.md#sys_req_interface_1); interface responses shall be outputs, not CLI commands.

| CLI command | Host request | Target output |
|---|---|---|
| `get-config` | [`GET_CONFIG_REQUEST`](sys_interfaces.md#get-config-request) | [`GET_CONFIG_RESPONSE`](sys_interfaces.md#get-config-response) |
| `set-config` | [`SET_CONFIG`](sys_interfaces.md#set-config-request) | - |
| `calibrate` | [`CALIBRATE`](sys_interfaces.md#calibrate-request) | - |
| `tx-packet` | [`TX_PACKET`](sys_interfaces.md#tx-packet-request) | - |
| `rx` | [`RX_START`](sys_interfaces.md#rx-start-request) | [`RX_PACKET`](sys_interfaces.md#rx-packet) |
| `cad` | [`CAD_START`](sys_interfaces.md#cad-start-request) | Outputs of the configured [CAD operation](sys_interfaces.md#cad-operation) |
| `tx-continuous-wave` | [`TX_CONTINUOUS_WAVE`](sys_interfaces.md#tx-continuous-wave-request) | - |
| `tx-continuous-preamble` | [`TX_CONTINUOUS_PREAMBLE`](sys_interfaces.md#tx-continuous-preamble-request) | - |
| `stop` | [`STOP`](sys_interfaces.md#stop-request) | - |
| `get-diagnostics` | [`GET_DIAGNOSTICS_REQUEST`](sys_interfaces.md#get-diagnostics-request) | [`GET_DIAGNOSTICS_RESPONSE`](sys_interfaces.md#get-diagnostics-response) |

Validation criteria:
- Verify that each listed command sends its mapped request.

Domain:
- sw

Upstream traceability:
- [SPEC_3](../../spec/specs.md#spec_3)
- [SPEC_4](../../spec/specs.md#spec_4)
- [SPEC_5](../../spec/specs.md#spec_5)
- [SPEC_6](../../spec/specs.md#spec_6)

References:
- [SYS_REQ_INTERFACE_1](sys_interfaces.md#sys_req_interface_1)

Status: approved

# SYS_REQ_CLI_3

The system shall map each applicable `sysp_*` request parameter to a long option by removing `sysp_`, converting camel-case to kebab-case, and prefixing `--`. A structure member shall append `-<member-name>` in kebab-case. The CLI shall parse, validate, and encode option values according to the applicable requirement in [sys_parameters.md](sys_parameters.md). Byte arrays and bit fields shall use contiguous lowercase hexadecimal with two digits per byte and no prefix or separator.

Validation criteria:
- For each applicable request parameter, verify its option name and the parsing and encoding of representative and boundary-valid values.
- Verify that invalid values, lengths, combinations, dependencies, and hexadecimal representations are rejected before request transmission.

Domain:
- sw

Upstream traceability:
- [SPEC_4](../../spec/specs.md#spec_4)
- [SPEC_5](../../spec/specs.md#spec_5)

References:
- [SYS_REQ_INTERFACE_1 Configuration Payload](sys_interfaces.md#configuration-payload)
- [SYS_REQ_INTERFACE_1 Calibration Payload](sys_interfaces.md#calibration-payload)
- [SYS_REQ_INTERFACE_1 TX Packet Payload](sys_interfaces.md#tx-packet-payload)

Status: approved

# SYS_REQ_CLI_4

The system shall reserve standard output for the records below and write all other output to standard error. Each standard-output event shall be one newline-terminated, space-separated `key=value` record with `event` first, snake_case keys, the specified field order, and no spaces in values. A parameter key shall remove `sysp_` and convert camel-case to snake_case; a structure member shall append `_<member-name>` in snake_case. Byte arrays and bit fields shall use the representation in [SYS_REQ_CLI_3](#sys_req_cli_3). The CLI shall emit `event=request_sent` only after transmitting the complete request; it shall not indicate remote acceptance, execution, or completion.

| Source | Output record field order |
|---|---|
| [`GET_CONFIG_RESPONSE`](sys_interfaces.md#get-config-response) | `event=config`, followed by the applicable [Configuration Payload](sys_interfaces.md#configuration-payload) parameter fields in payload order |
| [`RX_PACKET`](sys_interfaces.md#rx-packet) | `event=rx_packet`, followed by the [RX Packet Payload](sys_interfaces.md#rx-packet-payload) parameter fields in payload order |
| [`CAD_DETECTED`](sys_interfaces.md#cad-detected) | `event=cad_detected` |
| [`GET_DIAGNOSTICS_RESPONSE`](sys_interfaces.md#get-diagnostics-response) | `event=diagnostics`, followed by the [Diagnostics Payload](sys_interfaces.md#diagnostics-payload) parameter fields in payload order |
| Successful request without an interface response | `event=request_sent command=<CLI-command-name>` |

Validation criteria:
- For each table row, verify the standard-output record's stream, termination, field set, order, names, and value representation, and verify that other output is written to standard error.
- Parse each standard-output line by spaces and each field at its first `=`; verify a nonempty key and a value without spaces.

Domain:
- sw

Upstream traceability:
- [SPEC_3](../../spec/specs.md#spec_3)
- [SPEC_4](../../spec/specs.md#spec_4)
- [SPEC_5](../../spec/specs.md#spec_5)
- [SPEC_6](../../spec/specs.md#spec_6)

References:
- [SYS_REQ_INTERFACE_1](sys_interfaces.md#sys_req_interface_1)
- [SYS_REQ_CLI_3](#sys_req_cli_3)

Status: approved

# SYS_REQ_CLI_5

The system shall execute `rx` as a blocking command that outputs every report from the configured [RX operation](sys_interfaces.md#rx-operation), remains blocked while that operation is active, and terminates when it ends, is stopped, or is preempted. On Ctrl-C while RX is active, the CLI shall send [`STOP`](sys_interfaces.md#stop-request) before terminating.

Validation criteria:
- Exercise the packet and finite-timeout paths of the configured RX operation; verify report output and termination after a packet, and termination without output after a timeout.
- In `continuous` and `duty_cycle`, verify that multiple reports are output and the command remains blocked while RX is active.
- Stop and preempt active RX and verify termination; interrupt active RX with Ctrl-C and verify that `STOP` is sent before process termination.

Domain:
- sw

Upstream traceability:
- [SPEC_3](../../spec/specs.md#spec_3)
- [SPEC_5](../../spec/specs.md#spec_5)
- [SPEC_6](../../spec/specs.md#spec_6)

References:
- [SYS_REQ_INTERFACE_1](sys_interfaces.md#sys_req_interface_1)
- [SYS_REQ_CLI_4](#sys_req_cli_4)

Status: approved

# SYS_REQ_CLI_6

The system shall execute `cad` as a blocking command that outputs every report from the configured [CAD operation](sys_interfaces.md#cad-operation), remains blocked while that operation is active, and terminates when it ends, is stopped, or is preempted. On Ctrl-C while CAD is active, the CLI shall send [`STOP`](sys_interfaces.md#stop-request) before terminating.

Validation criteria:
- Exercise the `stop_on_detection` path and verify report output and command termination without `STOP`.
- Exercise the `receive_on_detection` packet and timeout paths; verify all produced reports, no packet output on timeout, and continued blocking after scanning resumes.
- Stop and preempt active CAD and verify termination; interrupt active CAD with Ctrl-C and verify that `STOP` is sent before process termination.

Domain:
- sw

Upstream traceability:
- [SPEC_3](../../spec/specs.md#spec_3)
- [SPEC_5](../../spec/specs.md#spec_5)
- [SPEC_6](../../spec/specs.md#spec_6)

References:
- [SYS_REQ_INTERFACE_1](sys_interfaces.md#sys_req_interface_1)
- [SYS_REQ_CLI_4](#sys_req_cli_4)

Status: approved

# SYS_REQ_CLI_7

The system shall return exit status zero when a command terminates successfully and nonzero when it fails or is interrupted by Ctrl-C.

Validation criteria:
- Verify that each successful command execution returns zero.
- Verify that a command failure and a Ctrl-C interruption each return nonzero without requiring a particular numeric value.

Domain:
- sw

Upstream traceability:
- [SPEC_3](../../spec/specs.md#spec_3)

References:
- [SYS_REQ_CLI_5](#sys_req_cli_5)
- [SYS_REQ_CLI_6](#sys_req_cli_6)

Status: approved
