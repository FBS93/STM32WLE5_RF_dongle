#!/usr/bin/env python3

# ==============================================================================
# @brief Development checks for the host CLI, without product hardware.
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
import binascii
import contextlib
import importlib.util
import io
import os
import pty
import signal
import struct
import subprocess
import sys
import termios
import time
import unittest
from pathlib import Path
from unittest import mock

# ------------------------------------------------------------------------------
# External library imports
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Project-specific imports
# ------------------------------------------------------------------------------
# Load the standalone script without installation or changes to sys.path.
spec = importlib.util.spec_from_file_location(
  "dongle_cli", Path(__file__).resolve().parents[1] / "cli.py"
)
cli = importlib.util.module_from_spec(spec)
spec.loader.exec_module(cli)

# ==============================================================================
# CONSTANTS
# ==============================================================================

# Independent wire fixtures in SYS_REQ_INTERFACE_1 field order.
COMMON = bytes.fromhex("000000403620f7")
GENERIC_FRAMING = bytes.fromhex(
  "10 123456789abcdef0 00 12 34 02 ffff 2110 01 ff01 02"
)
RX = bytes.fromhex("00 010000 000000 020000")
CONFIG_PAYLOADS = {
  "bpsk": b"\x02" + COMMON[1:] + bytes.fromhex("00409c 010000"),
  "msk": b"\x03"
  + COMMON[1:]
  + bytes.fromhex("e80300 00 0800 000000 01")
  + GENERIC_FRAMING,
  "fsk": COMMON
  + bytes.fromhex("e80300 001000 00 09 0800 000000 01 04")
  + GENERIC_FRAMING
  + RX,
  "lora": b"\x01"
  + COMMON[1:]
  + bytes.fromhex("07 04 01 00 0800 00 01 00 2414 000000 00 02")
  + bytes.fromhex("02 16 0a 01 010000")
  + RX,
}
COMMON_OUTPUT = (
  " rf_frequency_hz=868000000 payload_length_bytes=32 tx_output_power_dbm=-9"
)
GENERIC_OUTPUT = (
  " generic_sync_word_length_bits=16 generic_sync_word=123456789abcdef0"
  " generic_address_filtering=disabled generic_node_address=18"
  " generic_broadcast_address=52 generic_crc_type=crc16"
  " generic_crc_initial_value=65535 generic_crc_polynomial=4129"
  " generic_whitening_enabled=enabled generic_whitening_initial_value=511"
  " tx_ramp_time_us=40"
)
RX_OUTPUT = (
  " rx_mode=single rx_timeout_control=0.000015625"
  " rx_duty_cycle_rx_period=until_packet"
  " rx_duty_cycle_sleep_period=0.00003125"
)
CONFIG_OUTPUTS = {
  "bpsk": "event=config packet_type=bpsk"
  + COMMON_OUTPUT
  + " bpsk_bit_rate_bps=100 operation_timeout=0.000015625",
  "msk": "event=config packet_type=msk"
  + COMMON_OUTPUT
  + " generic_bit_rate_bps=1000 generic_pulse_shape=none preamble_length=8"
  + " operation_timeout=disabled generic_payload_length_mode=variable"
  + GENERIC_OUTPUT,
  "fsk": "event=config packet_type=fsk"
  + COMMON_OUTPUT
  + " generic_bit_rate_bps=1000 generic_frequency_deviation_hz=3906.25"
  + " generic_pulse_shape=none fsk_receive_bandwidth_k_hz=467"
  + " preamble_length=8 operation_timeout=disabled"
  + " generic_payload_length_mode=variable"
  + " generic_preamble_detection_length_bits=8"
  + GENERIC_OUTPUT
  + RX_OUTPUT,
  "lora": "event=config packet_type=lora"
  + COMMON_OUTPUT
  + " lora_spreading_factor=7 lora_bandwidth_k_hz=125 lora_coding_rate=4/5"
  + " lora_low_data_rate_optimize=disabled preamble_length=8"
  + " lora_header_mode=explicit lora_payload_crc_enabled=enabled"
  + " lora_invert_iq=standard lora_synchronization_word=private"
  + " operation_timeout=disabled lora_symbol_timeout=0 tx_ramp_time_us=40"
  + " cad_scan_symbol_count=4 cad_detection_peak_threshold=22"
  + " cad_detection_minimum_threshold=10 cad_exit_mode=receive_on_detection"
  + " cad_timeout=0.000015625"
  + RX_OUTPUT,
}

# Wire fixtures include independently fixed command IDs, payloads, CRC and COBS.
SEND_CASES = (
  (
    "calibrate",
    [
      "--image-calibration-lower-frequency-hz",
      "152000000",
      "--image-calibration-upper-frequency-hz",
      "960000000",
    ],
    "020202020526f0772500",
  ),
  ("tx-packet", ["--tx-packet-data", "0012abff"], "02030204010612abffa33c00"),
  ("tx-packet", ["--tx-packet-data", ""], "0203010103dc9b00"),
  ("tx-continuous-wave", [], "0206010103992700"),
  ("tx-continuous-preamble", [], "02070101032d5100"),
)

# ==============================================================================
# CLASSES
# ==============================================================================


##
# @brief Exercise wire primitives and CLI behavior with simulated serial I/O.
##
class CliTests(unittest.TestCase):
  ##
  # @brief Run main with captured streams and mocked POSIX transport calls.
  # @param[in] args CLI arguments including device selector.
  # @param[in] chunks Read side effects, including exceptions if needed.
  # @param[in] writes Write side effects, or None for complete writes.
  # @return Exit status, stdout, stderr and transmitted bytes.
  ##
  def run_cli(self, args, chunks=(), writes=None):
    stdout = io.StringIO()
    stderr = io.StringIO()
    transmitted = bytearray()
    write_results = iter(writes) if writes is not None else None

    ##
    # @brief Simulate an individual write, recording only accepted bytes.
    # @param[in] serial_fd Unused simulated descriptor.
    # @param[in] data Pending wire bytes.
    # @return Number of accepted bytes, or raises the injected exception.
    ##
    def write(serial_fd, data):
      result = next(write_results) if write_results is not None else len(data)
      if isinstance(result, BaseException):
        raise result
      transmitted.extend(data[:result])
      return result

    with (
      contextlib.redirect_stdout(stdout),
      contextlib.redirect_stderr(stderr),
      mock.patch.object(cli, "open_serial", return_value=42) as opened,
      mock.patch.object(cli.os, "close"),
      mock.patch.object(cli.os, "write", side_effect=write),
      mock.patch.object(cli.os, "read", side_effect=chunks),
      mock.patch.object(cli.select, "select", return_value=([42], [42], [])),
    ):
      status = cli.main(args)
      if status == 2:
        opened.assert_not_called()
    return status, stdout.getvalue(), stderr.getvalue(), bytes(transmitted)

  ##
  # @brief Verify known COBS and CRC vectors, frame layout and long blocks.
  ##
  def test_wire_vectors(self):
    self.assertEqual(binascii.crc_hqx(b"123456789", 0), 0x31C3)
    self.assertEqual(cli.cobs_encode(b"\x11\x00\x22"), b"\x02\x11\x02\x22")
    self.assertEqual(cli.encode_frame(0), bytes.fromhex("0101010101010100"))
    for data in (b"", b"\x00", bytes(range(256)), b"a" * 254, b"b" * 508):
      with self.subTest(length=len(data)):
        self.assertEqual(cli.cobs_decode(cli.cobs_encode(data)), data)
        frame = cli.encode_frame(0x8001, data)
        self.assertEqual(cli.decode_frame(frame[:-1]), (0x8001, data))
        body = cli.cobs_decode(frame[:-1])
        self.assertEqual(body[:4], struct.pack("<HH", 0x8001, len(data)))
        self.assertEqual(
          body[-2:], struct.pack("<H", binascii.crc_hqx(body[:-2], 0))
        )

  ##
  # @brief Reject invalid COBS, short frames, length mismatches and bad CRC.
  ##
  def test_invalid_frames(self):
    for body in (b"", b"\x00", b"\x03\x01", b"\x01", b"\x07abcdef"):
      with self.subTest(body=body), self.assertRaises(ValueError):
        cli.decode_frame(body)
    body = struct.pack("<HH", 0x8002, 4) + b"\x00\x00\x00"
    body += struct.pack("<H", binascii.crc_hqx(body, 0))
    with self.assertRaises(ValueError):
      cli.decode_frame(cli.cobs_encode(body))

  ##
  # @brief Verify full field order and values for all four configuration blocks.
  ##
  def test_configurations(self):
    for packet_type, payload in CONFIG_PAYLOADS.items():
      with self.subTest(packet_type=packet_type):
        frame = cli.encode_frame(0x8001, payload)
        result = self.run_cli(
          ["--device", "/fake", "get-config"],
          [bytes([value]) for value in frame],
        )
        self.assertEqual(
          result,
          (
            0,
            CONFIG_OUTPUTS[packet_type] + "\n",
            "",
            cli.encode_frame(0),
          ),
        )

  ##
  # @brief Verify diagnostics, fragmented/coalesced noise, and partial writes.
  ##
  def test_diagnostics_and_partial_writes(self):
    frame = cli.encode_frame(0x8002, bytes.fromhex("02 0101"))
    unrelated = cli.encode_frame(0x8004)
    invalid = cli.encode_frame(0x8002, b"\xff\x00\x00")
    chunks = [
      BlockingIOError(),
      InterruptedError(),
      b"\x00\x03\x00" + unrelated,
      invalid + frame[:2],
      frame[2:],
    ]
    status, stdout, stderr, sent = self.run_cli(
      ["--device", "/fake", "get-diagnostics", "--timeout", "0.5"],
      chunks,
      [BlockingIOError(), InterruptedError(), 1, 2, 5],
    )
    self.assertEqual(status, 0)
    self.assertEqual(
      stdout, "event=diagnostics active_rf_operation=rx radio_errors=0101\n"
    )
    self.assertEqual(stderr.count("Discarded frame:"), 2)
    self.assertEqual(sent, cli.encode_frame(9))

  ##
  # @brief STOP confirms complete transmission only, without reading a response.
  ##
  def test_stop(self):
    self.assertEqual(
      self.run_cli(["--device", "/fake", "stop"]),
      (
        0,
        "event=request_sent command=stop\n",
        "",
        cli.encode_frame(8),
      ),
    )
    status, stdout, stderr, _ = self.run_cli(
      ["--device", "/fake", "stop"],
      writes=[1, OSError("failed")],
    )
    self.assertEqual(status, 1)
    self.assertEqual(stdout, "")
    self.assertIn("failed", stderr)

  ##
  # @brief Check every new send command against fixed independent wire vectors.
  ##
  def test_send_commands(self):
    for command, options, wire in SEND_CASES:
      with self.subTest(command=command, options=options):
        self.assertEqual(
          self.run_cli(["--device", "/fake", command] + options),
          (
            0,
            f"event=request_sent command={command}\n",
            "",
            bytes.fromhex(wire),
          ),
        )

  ##
  # @brief Check calibration limits, exact encoding and interval ordering.
  ##
  def test_calibration_values(self):
    for encoded in range(0x26, 0xF1):
      self.assertEqual(
        cli.calibration_frequency(str(encoded * 4000000)), encoded
      )
    options = [
      "--image-calibration-lower-frequency-hz",
      "868000000",
      "--image-calibration-upper-frequency-hz",
      "868000000",
    ]
    self.assertEqual(
      self.run_cli(["--device", "/fake", "calibrate"] + options)[3],
      bytes.fromhex("0202020205d9d9c39300"),
    )
    invalid = []
    for index in (1, 3):
      for value in (
        "148000000",
        "964000000",
        "152000001",
        "151999999.9",
        "959999999.9",
        "0",
        "-1",
        "NaN",
        "sNaN",
        "Infinity",
        "bad",
      ):
        changed = options.copy()
        changed[index] = value
        invalid.append(changed)
    invalid += [[], options[:2], options[2:]]
    invalid.append(options[:3] + ["152000000"])
    for options in invalid:
      with self.subTest(options=options):
        result = self.run_cli(["--device", "/fake", "calibrate"] + options)
        self.assertEqual((result[0], result[1], result[3]), (2, "", b""))
        self.assertTrue(result[2])

  ##
  # @brief Check packet lengths, strict hex syntax and partial maximum writes.
  ##
  def test_packet_data(self):
    for size in (0, 1, 254, 255):
      payload = bytes(range(size))
      self.assertEqual(cli.packet_data(payload.hex()), payload)
    status, stdout, stderr, wire = self.run_cli(
      ["--device", "/fake", "tx-packet", "--tx-packet-data", "ab" * 255],
      writes=[BlockingIOError(), 1, InterruptedError(), 2, 260, 1],
    )
    self.assertEqual((status, stderr), (0, ""))
    self.assertEqual(stdout, "event=request_sent command=tx-packet\n")
    body = cli.cobs_decode(wire[:-1])
    self.assertEqual(body[:4], bytes.fromhex("0300ff00"))
    self.assertEqual(body[4:-2], b"\xab" * 255)
    for value in (
      "a",
      "ABCDEF",
      "0x00",
      "00 01",
      "00:01",
      "00\n",
      "gg",
      "00" * 256,
    ):
      with self.subTest(value=value):
        result = self.run_cli(
          ["--device", "/fake", "tx-packet", "--tx-packet-data", value]
        )
        self.assertEqual((result[0], result[1], result[3]), (2, "", b""))
        self.assertTrue(result[2])

  ##
  # @brief Check send failures/interruption never emit a success record.
  ##
  def test_send_failures(self):
    for command, options, _ in SEND_CASES:
      for error, expected in (
        (OSError("failed"), 1),
        (KeyboardInterrupt(), 130),
      ):
        with self.subTest(command=command, error=error):
          status, stdout, stderr, sent = self.run_cli(
            ["--device", "/fake", command] + options, writes=[1, error]
          )
          self.assertEqual((status, stdout, len(sent)), (expected, "", 1))
          self.assertTrue(stderr)
      with mock.patch.object(cli.time, "monotonic", side_effect=[0, 3]):
        status, stdout, stderr, sent = self.run_cli(
          ["--device", "/fake", command] + options
        )
      self.assertEqual((status, stdout, sent), (1, "", b""))
      self.assertIn("write timeout", stderr)

  ##
  # @brief Exercise direct script execution and all new request writes via PTY.
  ##
  def test_send_commands_pty(self):
    for command, options, wire in SEND_CASES:
      with self.subTest(command=command):
        master, slave = pty.openpty()
        try:
          process = subprocess.run(
            [
              sys.executable,
              "-B",
              str(Path(cli.__file__)),
              "--device",
              os.ttyname(slave),
              command,
            ]
            + options,
            capture_output=True,
            timeout=5,
            check=False,
          )
          self.assertEqual(process.returncode, 0, process.stderr)
          self.assertEqual(process.stderr, b"")
          self.assertEqual(
            process.stdout, f"event=request_sent command={command}\n".encode()
          )
          self.assertTrue(cli.select.select([master], [], [], 1)[0])
          self.assertEqual(os.read(master, 1024), bytes.fromhex(wire))
        finally:
          os.close(master)
          os.close(slave)

  ##
  # @brief Reject invalid arguments before access; send help to stderr.
  ##
  def test_arguments(self):
    invalid = [
      [],
      ["stop"],
      ["--device", "/fake", "unknown"],
      ["--device", "/fake", "stop", "--timeout", "1"],
      ["--dev", "/fake", "stop"],
      ["--device", "/fake", "tx-packet"],
      ["--device", "/fake", "tx-packet", "--tx-packet-d", "00"],
      ["--device", "/fake", "set-config"],
      ["--device", "/fake", "rx", "--rx-mode", "single"],
      ["--device", "/fake", "cad", "--timeout", "0"],
    ]
    invalid += [
      ["--device", "/fake", command] + options + ["--timeout", "1"]
      for command, options, _ in SEND_CASES
    ]
    invalid += [
      ["--device", "/fake", command, "--tx-packet-data", "00"]
      for command in ("tx-continuous-wave", "tx-continuous-preamble")
    ]
    invalid += [
      ["--device", "/fake", "get-config", "--timeout", value]
      for value in ("0", "-1", "nan", "inf", "bad")
    ]
    for args in invalid + [["--help"]]:
      with (
        self.subTest(args=args),
        mock.patch.object(cli, "open_serial") as opened,
      ):
        stdout, stderr = io.StringIO(), io.StringIO()
        with (
          contextlib.redirect_stdout(stdout),
          contextlib.redirect_stderr(stderr),
        ):
          status = cli.main(args)
        self.assertEqual(status, 0 if args == ["--help"] else 2)
        self.assertEqual(stdout.getvalue(), "")
        self.assertTrue(stderr.getvalue())
        opened.assert_not_called()

  ##
  # @brief Reject wrong configuration sizes and reserved response values.
  ##
  def test_invalid_payloads(self):
    for payload in (
      b"",
      b"\xff",
      CONFIG_PAYLOADS["lora"][:-1],
      CONFIG_PAYLOADS["msk"] + b"\x00",
    ):
      with self.subTest(payload=payload), self.assertRaises(ValueError):
        cli.response_record(0x8001, payload)
    for payload in (b"\x00", b"\x07\x00\x00", b"\x00\x80\x00"):
      with self.subTest(payload=payload), self.assertRaises(ValueError):
        cli.response_record(0x8002, payload)
    self.assertEqual(
      cli.format_value(b"\x00\x00\x00", "rx_timeout"), "no_timeout"
    )
    with self.assertRaises(ValueError):
      cli.format_value(b"\xff\xff\xff", "rx_timeout")

  ##
  # @brief Verify defaults and explicit timeout propagation to the query wait.
  ##
  def test_timeout_option(self):
    for command in ("get-config", "get-diagnostics"):
      for options, expected in (([], 2.0), (["--timeout", "0.125"], 0.125)):
        with (
          self.subTest(command=command, options=options),
          mock.patch.object(
            cli, "read_response", return_value="event=test"
          ) as read,
        ):
          self.assertEqual(
            self.run_cli(
              ["--device", "/fake", command] + options,
            )[0],
            0,
          )
          read.assert_called_once_with(42, cli.COMMANDS[command][1], expected)

  ##
  # @brief Check semantic enum/bitfield boundaries and exact fractional scaling.
  ##
  def test_parameter_values(self):
    for operation, name in enumerate(
      (
        "idle",
        "packet_tx",
        "rx",
        "cad",
        "continuous_wave_tx",
        "continuous_preamble_tx",
        "calibration",
      )
    ):
      self.assertEqual(
        cli.response_record(0x8002, bytes([operation, 1, 0])),
        f"event=diagnostics active_rf_operation={name} radio_errors=0001",
      )
    self.assertEqual(
      cli.format_value(b"\xaa\x0a\x1a", cli.CONFIG_FIELDS["bpsk"][0][2]), "600"
    )
    self.assertEqual(
      cli.format_value(b"\x01\x00\x60\x09", cli.FREQUENCY_FACTOR),
      "150000000.95367431640625",
    )
    self.assertEqual(
      cli.format_value(b"\xff\xff\xff", cli.TIME_FACTOR), "262.143984375"
    )
    for raw, kind in (
      (b"\xf6", "signed_power"),
      (b"\x17", "signed_power"),
      (b"\x41", (0, 64)),
    ):
      with self.subTest(raw=raw), self.assertRaises(ValueError):
        cli.format_value(raw, kind)

  ##
  # @brief Report open/configuration errors and close on receive failures.
  ##
  def test_cleanup_and_open_errors(self):
    for error in (OSError("open failed"), termios.error("not a terminal")):
      with (
        mock.patch.object(cli, "open_serial", side_effect=error),
        contextlib.redirect_stderr(io.StringIO()),
        contextlib.redirect_stdout(io.StringIO()) as stdout,
      ):
        self.assertEqual(cli.main(["--device", "/fake", "stop"]), 1)
        self.assertEqual(stdout.getvalue(), "")
    with (
      mock.patch.object(cli.os, "open", return_value=42),
      mock.patch.object(
        cli.termios, "tcgetattr", side_effect=termios.error("bad")
      ),
      mock.patch.object(cli.os, "close") as close,
    ):
      with self.assertRaises(termios.error):
        cli.open_serial("/fake")
      close.assert_called_once_with(42)
    with (
      mock.patch.object(cli, "open_serial", return_value=42),
      mock.patch.object(cli, "write_frame"),
      mock.patch.object(
        cli, "read_response", side_effect=OSError("read failed")
      ),
      mock.patch.object(cli.os, "close") as close,
      contextlib.redirect_stderr(io.StringIO()),
    ):
      self.assertEqual(cli.main(["--device", "/fake", "get-config"]), 1)
      close.assert_called_once_with(42)

  ##
  # @brief Bound waits across noise/retries; timeouts produce no records.
  ##
  def test_timeout(self):
    with mock.patch.object(cli.time, "monotonic", side_effect=[0, 0, 0, 0, 3]):
      status, stdout, stderr, sent = self.run_cli(
        ["--device", "/fake", "get-diagnostics"],
        [b"\x00"],
      )
    self.assertEqual((status, stdout, sent), (1, "", cli.encode_frame(9)))
    self.assertIn("response timeout", stderr)
    with mock.patch.object(cli.time, "monotonic", side_effect=[0, 3]):
      status, stdout, stderr, sent = self.run_cli(["--device", "/fake", "stop"])
    self.assertEqual((status, stdout, sent), (1, "", b""))
    self.assertIn("write timeout", stderr)

  ##
  # @brief Recover after oversized frames with bounded buffering.
  ##
  def test_oversized_frame(self):
    chunks = [
      b"a" * (cli.MAX_ENCODED_SIZE + 1),
      b"more\x00" + cli.encode_frame(0x8002, b"\x00\x00\x00"),
    ]
    status, stdout, stderr, _ = self.run_cli(
      ["--device", "/fake", "get-diagnostics"],
      chunks,
    )
    self.assertEqual(status, 0)
    self.assertIn("active_rf_operation=idle", stdout)
    self.assertIn("oversized", stderr)

  ##
  # @brief Handle disconnects, write no-progress and Ctrl-C as nonzero failures.
  ##
  def test_io_failures(self):
    for chunks in ([b""], [OSError("disconnected")], [KeyboardInterrupt()]):
      with self.subTest(chunks=chunks):
        status, stdout, stderr, _ = self.run_cli(
          ["--device", "/fake", "get-config"],
          chunks,
        )
        self.assertNotEqual(status, 0)
        self.assertEqual(stdout, "")
        self.assertTrue(stderr)
    self.assertEqual(
      self.run_cli(["--device", "/fake", "stop"], writes=[0])[0], 1
    )

  ##
  # @brief Check POSIX raw 115200 8N1 setup using a pseudo-terminal.
  ##
  def test_posix_settings(self):
    master, slave = pty.openpty()
    try:
      serial_fd = cli.open_serial(os.ttyname(slave))
      try:
        attrs = termios.tcgetattr(serial_fd)
        self.assertEqual(attrs[0:2], [0, 0])
        self.assertEqual(attrs[3], 0)
        self.assertEqual(attrs[4:6], [termios.B115200, termios.B115200])
        self.assertEqual(attrs[2] & termios.CSIZE, termios.CS8)
        self.assertFalse(attrs[2] & (termios.PARENB | termios.CSTOPB))
        cli.write_frame(serial_fd, cli.encode_frame(8))
        self.assertEqual(os.read(master, 64), cli.encode_frame(8))
      finally:
        os.close(serial_fd)
    finally:
      os.close(master)
      os.close(slave)

  ##
  # @brief Build options from independently written semantic fixture values.
  # @param[in] packet_type Fixture variant.
  # @param[in] changes Overrides or additional fields.
  # @return Complete command-line argument list.
  ##
  def config_args(self, packet_type, **changes):
    values = dict(
      item.split("=", 1) for item in CONFIG_OUTPUTS[packet_type].split()[1:]
    )
    values.update(changes)
    args = ["--device", "/fake", "set-config"]
    for name, value in values.items():
      if value is not None:
        args.extend(("--" + name.replace("_", "-"), str(value)))
    return args

  ##
  # @brief Check all SET_CONFIG variants against independent serialized blocks.
  ##
  def test_set_config_vectors(self):
    for packet_type, payload in CONFIG_PAYLOADS.items():
      with self.subTest(packet_type=packet_type):
        status, stdout, stderr, wire = self.run_cli(
          self.config_args(packet_type)
        )
        self.assertEqual(
          (status, stdout, stderr),
          (0, "event=request_sent command=set-config\n", ""),
        )
        body = cli.cobs_decode(wire[:-1])
        self.assertEqual(body[:4], struct.pack("<HH", 1, len(payload)))
        self.assertEqual(body[4:-2], payload)
        self.assertEqual(
          body[-2:], struct.pack("<H", binascii.crc_hqx(body[:-2], 0))
        )
    args = self.config_args("bpsk", bpsk_bit_rate_bps=600)
    self.assertEqual(
      cli.parse_arguments(args).payload[-6:-3], bytes.fromhex("aa0a1a")
    )

  ##
  # @brief Reject missing/foreign fields and hardware-independent dependencies.
  ##
  def test_set_config_invalid(self):
    cases = (
      ("bpsk", {"operation_timeout": None}),
      ("bpsk", {"rx_mode": "single"}),
      ("bpsk", {"bpsk_bit_rate_bps": "101"}),
      ("bpsk", {"rf_frequency_hz": "868000001"}),
      ("bpsk", {"operation_timeout": "0"}),
      ("fsk", {"generic_sync_word": "AABB"}),
      ("fsk", {"generic_sync_word": "00" * 7}),
      ("fsk", {"generic_sync_word": "00 " * 8}),
      (
        "fsk",
        {"payload_length_bytes": 255, "generic_address_filtering": "node"},
      ),
      (
        "msk",
        {
          "payload_length_bytes": 255,
          "generic_address_filtering": "node_and_broadcast",
        },
      ),
      (
        "fsk",
        {
          "generic_bit_rate_bps": 300000,
          "generic_frequency_deviation_hz": "199999.80926513671875",
        },
      ),
      ("fsk", {"fsk_receive_bandwidth_k_hz": "4.8"}),
      ("lora", {"lora_spreading_factor": 5}),
      ("lora", {"rf_frequency_hz": 150000000, "lora_bandwidth_k_hz": 250}),
      ("lora", {"rx_timeout_control": "262.143984375"}),
      (
        "lora",
        {"rx_mode": "duty_cycle", "rx_duty_cycle_rx_period": "0.000015625"},
      ),
      (
        "fsk",
        {"rx_mode": "duty_cycle", "rx_duty_cycle_rx_period": "0.000015625"},
      ),
    )
    for packet_type, changes in cases:
      with self.subTest(packet_type=packet_type, changes=changes):
        result = self.run_cli(self.config_args(packet_type, **changes))
        self.assertEqual((result[0], result[1], result[3]), (2, "", b""))
        self.assertTrue(result[2])
    for name in ("rf_frequency_hz", "operation_timeout", "tx_output_power_dbm"):
      for value in ("nan", "sNaN", "inf", "bad", "1.00000001", "-999"):
        with self.subTest(name=name, value=value):
          self.assertEqual(
            self.run_cli(self.config_args("bpsk", **{name: value}))[0], 2
          )

  ##
  # @brief Check every enum and range endpoint in configuration descriptors.
  ##
  def test_config_field_boundaries(self):
    for packet_type, fields in cli.CONFIG_FIELDS.items():
      for field in cli.COMMON_FIELDS + fields:
        name, size, kind = field
        if isinstance(kind, dict):
          for encoded, semantic in kind.items():
            with self.subTest(
              packet_type=packet_type, name=name, semantic=semantic
            ):
              self.assertEqual(
                cli.encode_value(str(semantic), field),
                encoded.to_bytes(size, "little"),
              )
        elif isinstance(kind, tuple):
          for value in kind:
            self.assertEqual(
              cli.encode_value(str(value), field),
              value.to_bytes(size, "little"),
            )
          for value in (kind[0] - 1, kind[1] + 1):
            with self.assertRaises(ValueError):
              cli.encode_value(str(value), field)
    cases = (
      (cli.COMMON_FIELDS[1], "150000000", 0x09600000),
      (cli.COMMON_FIELDS[1], "960000000", 0x3C000000),
      (cli.COMMON_FIELDS[3], "-9", 247),
      (cli.COMMON_FIELDS[3], "22", 22),
      (cli.CONFIG_FIELDS["fsk"][1], "600.8148193359375", 0x276),
      (cli.CONFIG_FIELDS["fsk"][1], "199999.80926513671875", 0x33333),
      (cli.OPERATION_TIMEOUT, "disabled", 0),
      (cli.OPERATION_TIMEOUT, "262.143984375", 0xFFFFFF),
      (cli.RX_FIELDS[1], "no_timeout", 0),
      (cli.RX_FIELDS[1], "262.14396875", 0xFFFFFE),
      (cli.RX_FIELDS[2], "until_packet", 0),
      (cli.RX_FIELDS[2], "262.143984375", 0xFFFFFF),
      (cli.RX_FIELDS[3], "0", 0),
    )
    for field, text, encoded in cases:
      with self.subTest(field=field[0], text=text):
        self.assertEqual(
          int.from_bytes(cli.encode_value(text, field), "little"), encoded
        )

  ##
  # @brief Accept boundary combinations and inactive fields of complete blocks.
  ##
  def test_config_valid_combinations(self):
    cases = (
      ("lora", {"lora_spreading_factor": 5, "preamble_length": 12}),
      ("lora", {"rf_frequency_hz": 400000000, "lora_bandwidth_k_hz": 500}),
      ("lora", {"rx_mode": "duty_cycle", "rx_duty_cycle_rx_period": "1"}),
      ("fsk", {"rx_mode": "duty_cycle", "rx_duty_cycle_rx_period": "1"}),
      ("fsk", {"rx_mode": "duty_cycle"}),
      (
        "msk",
        {"payload_length_bytes": 254, "generic_address_filtering": "node"},
      ),
      (
        "fsk",
        {"generic_crc_type": "none", "generic_whitening_enabled": "disabled"},
      ),
    )
    for packet_type, changes in cases:
      with self.subTest(packet_type=packet_type, changes=changes):
        self.assertEqual(
          self.run_cli(self.config_args(packet_type, **changes))[0], 0
        )

  ##
  # @brief Keep strict duty-window inequalities and reject sub-step fractions.
  ##
  def test_configuration_exact_constraints(self):
    for period, expected in (("0.012", 2), ("0.012015625", 0)):
      result = self.run_cli(
        self.config_args(
          "fsk",
          rx_mode="duty_cycle",
          rx_duty_cycle_rx_period=period,
          rx_duty_cycle_sleep_period="0",
        )
      )
      self.assertEqual(result[0], expected)
    for value in (
      "868000000.00000000000000000000000000000001",
      "149999999.99999999999999999999999999999999",
      "960000000.00000000000000000000000000000001",
    ):
      result = self.run_cli(self.config_args("bpsk", rf_frequency_hz=value))
      self.assertEqual((result[0], result[1], result[3]), (2, "", b""))

  ##
  # @brief Exercise SET_CONFIG as a direct script with independent PTY payloads.
  ##
  def test_set_config_pty_and_write_failures(self):
    for packet_type, payload in CONFIG_PAYLOADS.items():
      args = self.config_args(packet_type)
      for failure, expected in (
        (OSError("failed"), 1),
        (KeyboardInterrupt(), 130),
      ):
        result = self.run_cli(args, writes=[1, failure])
        self.assertEqual((result[0], result[1]), (expected, ""))
      master, slave = pty.openpty()
      try:
        args[1] = os.ttyname(slave)
        process = subprocess.run(
          [sys.executable, "-B", cli.__file__] + args,
          capture_output=True,
          timeout=5,
          check=False,
        )
        self.assertEqual((process.returncode, process.stderr), (0, b""))
        self.assertEqual(
          process.stdout, b"event=request_sent command=set-config\n"
        )
        self.assertEqual(self.pty_request(master), (1, payload))
      finally:
        os.close(master)
        os.close(slave)

  ##
  # @brief Run an RX/CAD simulation with immediate periodic diagnostic queries.
  # @param[in] command Blocking command name.
  # @param[in] config Configuration response payload.
  # @param[in] chunks Operation input fragments or injected exceptions.
  # @return Captured CLI result.
  ##
  def run_operation(self, command, config, chunks):
    with mock.patch.object(cli, "POLL_INTERVAL_SECONDS", 0):
      return self.run_cli(
        ["--device", "/fake", command, "--timeout", "0.5"],
        [cli.encode_frame(0x8001, config)] + chunks,
      )

  ##
  # @brief Verify exact signed LoRa/FSK reports and single-RX completion.
  ##
  def test_rx_single_reports(self):
    for packet_type, trailer, expected in (
      (
        "lora",
        "fff9",
        "received_packet_rssi_dbm=-127.5 received_packet_snr_db=-1.75",
      ),
      (
        "fsk",
        "01ff",
        "received_packet_rssi_dbm=-0.5 received_packet_status=ff",
      ),
    ):
      frame = cli.encode_frame(0x8003, b"\x02\x00\xff" + bytes.fromhex(trailer))
      result = self.run_operation(
        "rx", CONFIG_PAYLOADS[packet_type], [bytes([value]) for value in frame]
      )
      self.assertEqual(
        result[:3],
        (
          0,
          "event=rx_packet received_packet_length_bytes=2 rx_packet_data=00ff "
          + expected
          + "\n",
          "",
        ),
      )
      self.assertEqual(
        result[3],
        cli.encode_frame(0) + cli.encode_frame(4) + cli.encode_frame(9),
      )
    self.assertIn(
      "rx_packet_data= ", cli.packet_record(b"\x00\x00\x80", "lora")
    )
    for payload in (b"", b"\x00", b"\x02\x00\x00", b"\x00\x00\x00\x00"):
      with self.assertRaises(ValueError):
        cli.packet_record(payload, "lora")

  ##
  # @brief Follow continuous/duty RX across packets, idle and type preemption.
  ##
  def test_rx_modes_and_preemption(self):
    packet = cli.encode_frame(0x8003, bytes.fromhex("0100a080"))
    for packet_type in ("lora", "fsk"):
      for mode in (1, 2):
        config = bytearray(CONFIG_PAYLOADS[packet_type])
        config[-10] = mode
        for operation in (0, 1, 3, 4, 5, 6):
          with self.subTest(
            packet_type=packet_type, mode=mode, operation=operation
          ):
            chunks = [
              packet + packet + cli.encode_frame(0x8002, b"\x02\x00\x00"),
              cli.encode_frame(0x8002, bytes([operation, 0, 0])),
            ]
            status, stdout, stderr, wire = self.run_operation(
              "rx", bytes(config), chunks
            )
            self.assertEqual((status, stderr), (0, ""))
            self.assertEqual(stdout.count("event=rx_packet"), 2)
            self.assertNotIn("diagnostics", stdout)
            self.assertEqual(wire.count(cli.encode_frame(9)), 2)
    result = self.run_operation(
      "rx", CONFIG_PAYLOADS["lora"], [cli.encode_frame(0x8002, b"\x00\x00\x00")]
    )
    self.assertEqual(result[:3], (0, "", ""))

  ##
  # @brief CAD detection ends only stop mode; receive mode resumes scanning.
  ##
  def test_cad_modes(self):
    config = bytearray(CONFIG_PAYLOADS["lora"])
    config[26] = 0
    detected = cli.encode_frame(0x8004)
    result = self.run_operation("cad", bytes(config), [detected])
    self.assertEqual(result[:3], (0, "event=cad_detected\n", ""))
    self.assertNotIn(cli.encode_frame(8), result[3])
    packet = cli.encode_frame(0x8003, bytes.fromhex("0100a080"))
    active = cli.encode_frame(0x8002, b"\x03\x00\x00")
    result = self.run_operation(
      "cad",
      CONFIG_PAYLOADS["lora"],
      [
        active,
        detected + packet + active,
        detected + active,
        cli.encode_frame(0x8002, b"\x02\x00\x00"),
      ],
    )
    self.assertEqual((result[0], result[2]), (0, ""))
    self.assertEqual(result[1].count("event=cad_detected"), 2)
    self.assertEqual(result[1].count("event=rx_packet"), 1)
    self.assertNotIn(cli.encode_frame(8), result[3])

  ##
  # @brief Verify STOP on interruption, errors and invalid reports/queries.
  ##
  def test_operation_failures(self):
    for command in ("rx", "cad"):
      result = self.run_operation(
        command, CONFIG_PAYLOADS["lora"], [KeyboardInterrupt()]
      )
      self.assertEqual(result[0:2], (130, ""))
      self.assertTrue(result[3].endswith(cli.encode_frame(8)))
      for failure in (
        OSError("disconnected"),
        TimeoutError("response timeout"),
        b"",
        cli.encode_frame(0x8002, b"\x00\x01\x00"),
      ):
        result = self.run_operation(command, CONFIG_PAYLOADS["lora"], [failure])
        self.assertEqual(result[0:2], (1, ""))
        self.assertTrue(result[2])
      malformed = (
        cli.encode_frame(0x8003, b"\x02\x00\x00")
        + cli.encode_frame(0x8004, b"\x01")
        + cli.encode_frame(0x8002, b"\x00\x00\x00")
      )
      result = self.run_operation(command, CONFIG_PAYLOADS["lora"], [malformed])
      self.assertEqual(result[:2], (0, ""))
      self.assertIn("Discarded frame", result[2])
    for command, packet_type in (("rx", "bpsk"), ("rx", "msk"), ("cad", "fsk")):
      result = self.run_operation(command, CONFIG_PAYLOADS[packet_type], [])
      self.assertEqual(result[0:2], (1, ""))
      self.assertEqual(result[3], cli.encode_frame(0))

  ##
  # @brief Read one subprocess request from a PTY with a bounded test deadline.
  # @param[in] master PTY master descriptor.
  # @return Decoded request ID and payload.
  ##
  def pty_request(self, master):
    wire = bytearray()
    deadline = time.monotonic() + 5
    while not wire or wire[-1] != 0:
      self.assertTrue(
        cli.select.select(
          [master], [], [], max(0, deadline - time.monotonic())
        )[0]
      )
      wire.extend(os.read(master, 1))
    return cli.decode_frame(wire[:-1])

  ##
  # @brief Exercise real blocking, silent completion, query timeout and SIGINT.
  ##
  def test_blocking_operations_pty(self):
    for command, ending in (
      ("rx", "idle"),
      ("rx", "packet"),
      ("cad", "interrupt"),
      ("cad", "timeout"),
    ):
      with self.subTest(command=command, ending=ending):
        master, slave = pty.openpty()
        process = subprocess.Popen(
          [
            sys.executable,
            "-B",
            cli.__file__,
            "--device",
            os.ttyname(slave),
            command,
            "--timeout",
            "0.15",
          ],
          stdout=subprocess.PIPE,
          stderr=subprocess.PIPE,
        )
        try:
          self.assertEqual(self.pty_request(master), (0, b""))
          os.write(master, cli.encode_frame(0x8001, CONFIG_PAYLOADS["lora"]))
          self.assertEqual(
            self.pty_request(master), (4 if command == "rx" else 5, b"")
          )
          time.sleep(0.05)
          self.assertIsNone(process.poll(), "silence must not end RX/CAD")
          if ending == "packet":
            frame = cli.encode_frame(0x8003, b"\x00\xff\x80")
            for value in frame:
              os.write(master, bytes([value]))
          else:
            self.assertEqual(self.pty_request(master), (9, b""))
            if ending == "idle":
              os.write(master, cli.encode_frame(0x8002, b"\x00\x00\x00"))
            elif ending == "interrupt":
              os.write(master, cli.encode_frame(0x8002, b"\x03\x00\x00"))
              time.sleep(0.2)
              self.assertIsNone(
                process.poll(), "query timeout is not a lifetime"
              )
              os.kill(process.pid, signal.SIGINT)
              self.assertEqual(self.pty_request(master), (8, b""))
          stdout, stderr = process.communicate(timeout=5)
          self.assertEqual(
            process.returncode,
            {"idle": 0, "packet": 0, "interrupt": 130, "timeout": 1}[ending],
          )
          if ending == "packet":
            self.assertTrue(stdout.startswith(b"event=rx_packet "))
          else:
            self.assertEqual(stdout, b"")
          self.assertEqual(bool(stderr), ending in ("interrupt", "timeout"))
        finally:
          if process.poll() is None:
            process.kill()
          process.communicate()
          os.close(master)
          os.close(slave)


# ==============================================================================
# FUNCTIONS
# ==============================================================================

# ==============================================================================
# SCRIPT ENTRY POINT
# ==============================================================================

if __name__ == "__main__":
  unittest.main()
