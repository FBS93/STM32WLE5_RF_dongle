# Run target serial logging server overview

`run_target_serial_logging_server.py` runs on the Raspberry Pi and exposes one target serial interface as a TCP logging bridge so host-side tools can observe logs and send serial input remotely.

It works as follows:
- Opening the specified serial device in raw mode.
- Binding a TCP server on the specified port.
- Accepting multiple concurrent TCP clients.
- Broadcasting target serial RX data to all connected TCP clients.
- Forwarding TCP TX data from any connected client to the serial interface.

# Glossary

| Term | Definition |
|---|---|
| Serial device | Linux character device representing the USB-to-serial adapter connected to the target. |
| Logging bridge | Process that forwards serial RX data to TCP clients and forwards TCP TX data back to the serial interface. |
| TCP client | Host-side consumer connected to the logging port to receive target output or send input. |

# Usage example

Arguments:
- `<serial_device>`: Absolute path to the serial device to open.
- `<tcp_port>`: TCP port exposed for the logging interface.
- `<baud_rate>`: Serial baud rate.

Run the logging bridge directly on the Raspberry Pi:

```bash
python tools/run_target_serial_logging_server/run_target_serial_logging_server.py /dev/serial/by-id/usb-debug-adapter 3334 115200
```
