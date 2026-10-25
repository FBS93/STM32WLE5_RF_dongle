# CLI overview

`cli.py` controls the RF dongle from a POSIX host using Python 3.11 or newer and the standard library. Run it directly; no installed executable or launcher is provided.

See the [CLI requirements](../../sys/req/sys_cli.md) for commands, parameters and output records, and the [protocol](../../sys/req/sys_interfaces.md#sys_req_interface_1) for payloads. The host CLI scope is defined by [SW_ARCH_DESIGN_3](../arch/sw_designs.md#sw_arch_design_3).

# Glossary

| Term | Definition |
|---|---|
| Response timeout | Host per-query waiting limit, not an RF operation timeout. |

# Usage example

From the repository root, replacing `/dev/ttyUSB0` with the product's Virtual COM Port:

```bash
python sw/cli/cli.py --device /dev/ttyUSB0 get-config --timeout 5
```

Use `python sw/cli/cli.py --help` to list commands and `python sw/cli/cli.py --device /dev/ttyUSB0 set-config --help` for configuration options. `set-config` requires every field in the [common prefix and selected configuration block](../../sys/req/sys_interfaces.md#configuration-payload), including inactive fields; there are no implicit RF defaults or partial updates.

Usage notes:
- `--timeout SECONDS` is available on queries and `rx`/`cad`: finite and positive, default 2 seconds. Each query deadline starts after request transmission and is not extended by incoming fragments or unrelated records. For `rx`/`cad`, it is not an operation or inactivity limit: a quiet active operation can block indefinitely. Writes have a separate fixed 2-second guard; requests are not automatically retried.
- On a host timeout or transport failure, RF stoppage is not guaranteed; restore access and use `stop` if needed. Ctrl-C during `rx`/`cad` attempts STOP before exiting.
- Use one process per device with no pending requests. The protocol cannot identify stale responses or distinguish replacement by another RF operation of the same type between diagnostic polls.
- The CLI sends requested values without oscillator compensation. The [HSE-error-dependent FSK bound](../../sys/req/sys_parameters.md#sys_req_parameter_11) is left to firmware.

