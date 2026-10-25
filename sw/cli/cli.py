#!/usr/bin/env python3

# ==============================================================================
# @brief POSIX host CLI for the STM32WLE5 RF dongle.
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
import argparse
import binascii
import contextlib
import math
import os
import select
import struct
import sys
import termios
import time
from decimal import Decimal
from fractions import Fraction

# ------------------------------------------------------------------------------
# External library imports
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Project-specific imports
# ------------------------------------------------------------------------------

# ==============================================================================
# CONSTANTS
# ==============================================================================

# Local host policies, not timing requirements on the target.
DEFAULT_TIMEOUT_SECONDS = 2.0
WRITE_TIMEOUT_SECONDS = 2.0
POLL_INTERVAL_SECONDS = 0.5

# Maximum encoded frame without delimiter: uint16 payload plus header and CRC.
MAX_DECODED_SIZE = 65535 + 6
MAX_ENCODED_SIZE = MAX_DECODED_SIZE + MAX_DECODED_SIZE // 254 + 1

# Request and expected response IDs; None means no interface response exists.
COMMANDS = {
  "get-config": (0x0000, 0x8001),
  "set-config": (0x0001, None),
  "calibrate": (0x0002, None),
  "tx-packet": (0x0003, None),
  "rx": (0x0004, 0x8003),
  "cad": (0x0005, 0x8004),
  "tx-continuous-wave": (0x0006, None),
  "tx-continuous-preamble": (0x0007, None),
  "stop": (0x0008, None),
  "get-diagnostics": (0x0009, 0x8002),
}

# Enumeration tables preserve the semantic values of SYS_REQ_PARAMETER_*.
PACKET_TYPES = {0: "fsk", 1: "lora", 2: "bpsk", 3: "msk"}
ENABLED = {0: "disabled", 1: "enabled"}
PULSE_SHAPES = {
  0: "none",
  8: "gaussian_bt_0_3",
  9: "gaussian_bt_0_5",
  10: "gaussian_bt_0_7",
  11: "gaussian_bt_1_0",
}
LORA_BANDWIDTHS = {
  0: "7.81",
  8: "10.42",
  1: "15.63",
  9: "20.83",
  2: "31.25",
  10: "41.67",
  3: "62.5",
  4: "125",
  5: "250",
  6: "500",
}
FSK_BANDWIDTHS = {
  0x1F: "4.8",
  0x17: "5.8",
  0x0F: "7.3",
  0x1E: "9.7",
  0x16: "11.7",
  0x0E: "14.6",
  0x1D: "19.5",
  0x15: "23.4",
  0x0D: "29.3",
  0x1C: "39",
  0x14: "46.9",
  0x0C: "58.6",
  0x1B: "78.2",
  0x13: "93.8",
  0x0B: "117.3",
  0x1A: "156.2",
  0x12: "187.2",
  0x0A: "234.3",
  0x19: "312",
  0x11: "373.6",
  0x09: "467",
}
RAMP_TIMES = dict(enumerate((10, 20, 40, 80, 200, 800, 1700, 3400)))
ACTIVE_OPERATIONS = dict(
  enumerate(
    (
      "idle",
      "packet_tx",
      "rx",
      "cad",
      "continuous_wave_tx",
      "continuous_preamble_tx",
      "calibration",
    )
  )
)

# Exact physical-unit factors; multibyte integers are little-endian.
FREQUENCY_FACTOR = Decimal(32000000) / Decimal(2**25)
TIME_FACTOR = Decimal("0.000015625")

# Field descriptors: output key, byte size, interpretation. A tuple is a range;
# a dict is an enum; Decimal is a scale; strings select the special formats.
COMMON_FIELDS = (
  ("packet_type", 1, PACKET_TYPES),
  ("rf_frequency_hz", 4, FREQUENCY_FACTOR),
  ("payload_length_bytes", 1, (0, 255)),
  ("tx_output_power_dbm", 1, "signed_power"),
)
OPERATION_TIMEOUT = ("operation_timeout", 3, "operation_timeout")
RAMP_TIME = ("tx_ramp_time_us", 1, RAMP_TIMES)
RX_FIELDS = (
  ("rx_mode", 1, {0: "single", 1: "continuous", 2: "duty_cycle"}),
  ("rx_timeout_control", 3, "rx_timeout"),
  ("rx_duty_cycle_rx_period", 3, "rx_period"),
  ("rx_duty_cycle_sleep_period", 3, TIME_FACTOR),
)
GENERIC_FRAMING_FIELDS = (
  ("generic_sync_word_length_bits", 1, (0, 64)),
  ("generic_sync_word", 8, "bytes"),
  (
    "generic_address_filtering",
    1,
    {0: "disabled", 1: "node", 2: "node_and_broadcast"},
  ),
  ("generic_node_address", 1, (0, 255)),
  ("generic_broadcast_address", 1, (0, 255)),
  (
    "generic_crc_type",
    1,
    {0: "crc8", 1: "none", 2: "crc16", 4: "crc8_inverted", 6: "crc16_inverted"},
  ),
  ("generic_crc_initial_value", 2, (0, 65535)),
  ("generic_crc_polynomial", 2, (0, 65535)),
  ("generic_whitening_enabled", 1, ENABLED),
  ("generic_whitening_initial_value", 2, (0, 511)),
  RAMP_TIME,
)
PAYLOAD_LENGTH_MODE = (
  "generic_payload_length_mode",
  1,
  {0: "fixed", 1: "variable"},
)
CONFIG_FIELDS = {
  "lora": (
    ("lora_spreading_factor", 1, (5, 12)),
    ("lora_bandwidth_k_hz", 1, LORA_BANDWIDTHS),
    (
      "lora_coding_rate",
      1,
      dict(enumerate(("4/4", "4/5", "4/6", "4/7", "4/8"))),
    ),
    ("lora_low_data_rate_optimize", 1, ENABLED),
    ("preamble_length", 2, (1, 65535)),
    ("lora_header_mode", 1, {0: "explicit", 1: "implicit"}),
    ("lora_payload_crc_enabled", 1, ENABLED),
    ("lora_invert_iq", 1, {0: "standard", 1: "inverted"}),
    ("lora_synchronization_word", 2, {0x1424: "private", 0x3444: "public"}),
    OPERATION_TIMEOUT,
    ("lora_symbol_timeout", 1, (0, 255)),
    RAMP_TIME,
    ("cad_scan_symbol_count", 1, dict(enumerate((1, 2, 4, 8, 16)))),
    ("cad_detection_peak_threshold", 1, (0, 255)),
    ("cad_detection_minimum_threshold", 1, (0, 255)),
    ("cad_exit_mode", 1, {0: "stop_on_detection", 1: "receive_on_detection"}),
    ("cad_timeout", 3, TIME_FACTOR),
  )
  + RX_FIELDS,
  "fsk": (
    ("generic_bit_rate_bps", 3, (600, 300000)),
    ("generic_frequency_deviation_hz", 3, FREQUENCY_FACTOR),
    ("generic_pulse_shape", 1, PULSE_SHAPES),
    ("fsk_receive_bandwidth_k_hz", 1, FSK_BANDWIDTHS),
    ("preamble_length", 2, (8, 65535)),
    OPERATION_TIMEOUT,
    PAYLOAD_LENGTH_MODE,
    (
      "generic_preamble_detection_length_bits",
      1,
      {0: "disabled", 4: 8, 5: 16, 6: 24, 7: 32},
    ),
  )
  + GENERIC_FRAMING_FIELDS
  + RX_FIELDS,
  "msk": (
    ("generic_bit_rate_bps", 3, (100, 10000)),
    ("generic_pulse_shape", 1, PULSE_SHAPES),
    ("preamble_length", 2, (8, 65535)),
    OPERATION_TIMEOUT,
    PAYLOAD_LENGTH_MODE,
  )
  + GENERIC_FRAMING_FIELDS,
  "bpsk": (
    ("bpsk_bit_rate_bps", 3, {0x9C4000: 100, 0x1A0AAA: 600}),
    OPERATION_TIMEOUT,
  ),
}
DIAGNOSTICS_FIELDS = (
  ("active_rf_operation", 1, ACTIVE_OPERATIONS),
  ("radio_errors", 2, "bits"),
)

# ==============================================================================
# CLASSES
# ==============================================================================


##
# @brief Retain serial fragments and coalesced frames across response waits.
##
class FrameReader:
  ##
  # @brief Initialize bounded input state.
  # @param[in] serial_fd Open serial descriptor.
  ##
  def __init__(self, serial_fd):
    self.serial_fd = serial_fd
    self.pending = bytearray()
    self.encoded = bytearray()
    self.oversized = False

  ##
  # @brief Read a validated frame without renewing the absolute deadline.
  # @param[in] deadline Absolute monotonic read deadline.
  # @return Command ID and payload; invalid frames are discarded.
  ##
  def read(self, deadline):
    while True:
      if not self.pending:
        wait_ready(self.serial_fd, False, deadline)
        try:
          chunk = os.read(self.serial_fd, 4096)
        except (BlockingIOError, InterruptedError):
          continue
        if not chunk:
          raise OSError("serial device disconnected")
        self.pending.extend(chunk)

      value = self.pending.pop(0)
      if value == 0:
        # Reset assembly state before validating this delimited frame.
        encoded = bytes(self.encoded)
        self.encoded.clear()
        oversized = self.oversized
        self.oversized = False

        if encoded and not oversized:
          try:
            return decode_frame(encoded)
          except ValueError as error:
            print(f"Discarded frame: {error}", file=sys.stderr)
      elif not self.oversized:
        self.encoded.append(value)
        if len(self.encoded) > MAX_ENCODED_SIZE:
          # Ignore the remaining bytes until a delimiter restores framing.
          self.encoded.clear()
          self.oversized = True
          print("Discarded oversized frame", file=sys.stderr)


# ==============================================================================
# FUNCTIONS
# ==============================================================================


##
# @brief COBS-encode bytes, without the frame delimiter.
# @param[in] data Unencoded bytes.
# @return Encoded bytes.
##
def cobs_encode(data):
  encoded = bytearray(b"\x00")
  code_index = 0
  code = 1

  for value in data:
    if value == 0:
      encoded[code_index] = code
      code_index = len(encoded)
      encoded.append(0)
      code = 1
    else:
      encoded.append(value)
      code += 1
      if code == 255:
        encoded[code_index] = code
        code_index = len(encoded)
        encoded.append(0)
        code = 1

  encoded[code_index] = code
  return bytes(encoded)


##
# @brief Decode one COBS body, rejecting zero bytes and truncated blocks.
# @param[in] encoded Encoded bytes without delimiter.
# @return Decoded bytes; raises ValueError for invalid encoding.
##
def cobs_decode(encoded):
  if not encoded or 0 in encoded:
    raise ValueError("invalid COBS body")

  decoded = bytearray()
  offset = 0

  while offset < len(encoded):
    code = encoded[offset]
    end = offset + code
    if end > len(encoded):
      raise ValueError("truncated COBS block")

    decoded.extend(encoded[offset + 1 : end])
    offset = end
    # A full 254-byte block does not imply a trailing zero.
    if code != 255 and offset < len(encoded):
      decoded.append(0)

  return bytes(decoded)


##
# @brief Serialize a protocol frame with CRC-16/XMODEM and delimiter.
# @param[in] command_id Protocol command ID.
# @param[in] payload Payload bytes.
# @return Complete wire frame.
##
def encode_frame(command_id, payload=b""):
  body = struct.pack("<HH", command_id, len(payload)) + payload
  body += struct.pack("<H", binascii.crc_hqx(body, 0))
  return cobs_encode(body) + b"\x00"


##
# @brief Check a received frame's format, exact length and CRC.
# @param[in] encoded COBS body without delimiter.
# @return Tuple (command ID, payload); raises ValueError for invalid frames.
##
def decode_frame(encoded):
  body = cobs_decode(encoded)
  if len(body) < 6:
    raise ValueError("short frame")

  command_id, length = struct.unpack_from("<HH", body)
  if len(body) != length + 6:
    raise ValueError("payload length mismatch")
  if binascii.crc_hqx(body[:-2], 0) != int.from_bytes(body[-2:], "little"):
    raise ValueError("CRC mismatch")

  return command_id, body[4:-2]


##
# @brief Decode one parameter into its CLI semantic representation.
# @param[in] raw Serialized field bytes.
# @param[in] kind Field interpretation from a descriptor.
# @return Value without spaces; raises ValueError for reserved values.
##
def format_value(raw, kind):
  value = int.from_bytes(raw, "little")

  if isinstance(kind, dict):
    if value not in kind:
      raise ValueError("reserved enumeration value")
    return str(kind[value])

  if isinstance(kind, tuple):
    if not kind[0] <= value <= kind[1]:
      raise ValueError("parameter out of range")
    return str(value)

  if kind == "bytes":
    return raw.hex()
  if kind == "bits":
    if value & ~0x017F:
      raise ValueError("reserved radio error bits")
    return f"{value:04x}"

  if kind == "signed_power":
    value = int.from_bytes(raw, "little", signed=True)
    if not -9 <= value <= 22:
      raise ValueError("TX power out of range")
    return str(value)

  if kind in ("operation_timeout", "rx_timeout", "rx_period"):
    if value == 0:
      return {
        "operation_timeout": "disabled",
        "rx_timeout": "no_timeout",
        "rx_period": "until_packet",
      }[kind]
    if kind == "rx_timeout" and value == 0xFFFFFF:
      raise ValueError("reserved RX timeout")
    kind = TIME_FACTOR

  return format(value * kind, "f").rstrip("0").rstrip(".") or "0"


##
# @brief Build a complete response record, never returning a partial record.
# @param[in] response_id Expected response ID.
# @param[in] payload Serialized response payload.
# @return Newline-free event record; raises ValueError for invalid payloads.
##
def response_record(response_id, payload):
  if response_id == 0x8001:
    if not payload or payload[0] not in PACKET_TYPES:
      raise ValueError("invalid packet type")
    fields = COMMON_FIELDS + CONFIG_FIELDS[PACKET_TYPES[payload[0]]]
    event = "config"
  else:
    fields = DIAGNOSTICS_FIELDS
    event = "diagnostics"

  if len(payload) != sum(size for _, size, _ in fields):
    raise ValueError("response payload size mismatch")

  record = [f"event={event}"]
  offset = 0
  for name, size, kind in fields:
    raw = payload[offset : offset + size]
    value = int.from_bytes(raw, "little")
    if name == "rf_frequency_hz" and not 0x09600000 <= value <= 0x3C000000:
      raise ValueError("RF frequency out of range")
    if (
      name == "generic_frequency_deviation_hz" and not 0x276 <= value <= 0x33333
    ):
      raise ValueError("frequency deviation out of range")

    record.append(f"{name}={format_value(raw, kind)}")
    offset += size

  return " ".join(record)


##
# @brief Open nonblocking raw 115200 8N1 serial, without flow control.
# @param[in] device Serial device path.
# @return Open serial descriptor; the device remains configured after closing.
##
def open_serial(device):
  serial_fd = os.open(device, os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK)
  try:
    attrs = termios.tcgetattr(serial_fd)
    attrs[0] = 0
    attrs[1] = 0
    attrs[2] = termios.CS8 | termios.CREAD | termios.CLOCAL
    attrs[3] = 0
    attrs[4] = termios.B115200
    attrs[5] = termios.B115200
    attrs[6][termios.VMIN] = 1
    attrs[6][termios.VTIME] = 0
    termios.tcsetattr(serial_fd, termios.TCSANOW, attrs)

    return serial_fd
  except BaseException:
    os.close(serial_fd)
    raise


##
# @brief Wait for serial readiness without extending an absolute deadline.
# @param[in] serial_fd Open descriptor.
# @param[in] writing True for write readiness, False for read readiness.
# @param[in] deadline Absolute monotonic time limit.
##
def wait_ready(serial_fd, writing, deadline):
  while True:
    remaining = deadline - time.monotonic()
    if remaining <= 0:
      raise TimeoutError(
        "serial write timeout" if writing else "response timeout"
      )

    try:
      # Slice large waits to avoid platform select() timeout overflow.
      readable, writable, _ = select.select(
        [] if writing else [serial_fd],
        [serial_fd] if writing else [],
        [],
        min(remaining, 1.0),
      )
    except InterruptedError:
      continue

    if readable or writable:
      return


##
# @brief Send the complete frame, handling partial writes and transient errors.
# @param[in] serial_fd Open serial descriptor.
# @param[in] frame Complete wire frame.
##
def write_frame(serial_fd, frame):
  deadline = time.monotonic() + WRITE_TIMEOUT_SECONDS
  offset = 0

  while offset < len(frame):
    wait_ready(serial_fd, True, deadline)
    try:
      written = os.write(serial_fd, frame[offset:])
    except (BlockingIOError, InterruptedError):
      continue

    if written == 0:
      raise OSError("serial write made no progress")
    offset += written


##
# @brief Receive the expected response across arbitrary serial fragments.
# @param[in] serial_fd Open serial descriptor.
# @param[in] response_id Expected response ID; other IDs are ignored.
# @param[in] timeout Response wait in seconds after request transmission.
# @return Complete validated response record.
##
def read_response(serial_fd, response_id, timeout):
  deadline = time.monotonic() + timeout
  reader = FrameReader(serial_fd)

  while True:
    command_id, payload = reader.read(deadline)
    try:
      if command_id == response_id:
        return response_record(response_id, payload)
    except ValueError as error:
      print(f"Discarded frame: {error}", file=sys.stderr)


##
# @brief Encode one configuration option without rounding or HW offsets.
# @param[in] text Physical or named value.
# @param[in] field Field descriptor.
# @return Serialized field bytes; raises ValueError for invalid input.
##
def encode_value(text, field):
  name, size, kind = field

  if kind == "bytes":
    raw = packet_data(text)
    if len(raw) != size:
      raise ValueError(f"{name} requires exactly {size} bytes")
    return raw

  if isinstance(kind, dict):
    for value, semantic in kind.items():
      if text == str(semantic):
        return value.to_bytes(size, "little")
      if isinstance(semantic, (int, float)) or str(semantic)[0].isdigit():
        try:
          if Decimal(text) == Decimal(str(semantic)):
            return value.to_bytes(size, "little")
        except ArithmeticError:
          pass
    raise ValueError(f"{name}: expected one of {list(kind.values())}")

  # Zero-valued time sentinels must use their names, not numeric aliases.
  named = {
    "operation_timeout": "disabled",
    "rx_timeout": "no_timeout",
    "rx_period": "until_packet",
  }
  if isinstance(kind, str) and kind in named:
    if text == named[kind]:
      return bytes(size)
    kind = TIME_FACTOR
    minimum = 1
  else:
    minimum = 0

  try:
    physical = Decimal(text)
    if not physical.is_finite():
      raise ValueError("value must be finite")

    # Rational scaling preserves sub-step input that Decimal could round away.
    value = Fraction(physical)
    if isinstance(kind, Decimal):
      value /= Fraction(kind)
    if value.denominator != 1:
      raise ValueError("value must be exactly representable; no rounding")
    value = int(value)

    maximum = (1 << (8 * size)) - 1
    if isinstance(kind, tuple):
      minimum, maximum = kind
    if kind == "signed_power":
      minimum, maximum = -9, 22
    if name == "rf_frequency_hz":
      minimum, maximum = 0x09600000, 0x3C000000
    if name == "generic_frequency_deviation_hz":
      minimum, maximum = 0x276, 0x33333
    if name == "rx_timeout_control":
      maximum = 0xFFFFFE
    if not minimum <= value <= maximum:
      raise ValueError("value out of range")

    return value.to_bytes(size, "little", signed=kind == "signed_power")
  except ArithmeticError as error:
    raise ValueError("invalid numeric value") from error


##
# @brief Check hardware-independent constraints on a complete configuration.
# @param[in] values Semantic values decoded from the serialized fields.
##
def validate_configuration(values):
  packet_type = values["packet_type"]
  if packet_type == "lora":
    if (
      Decimal(values["rf_frequency_hz"]) < 400000000
      and Decimal(values["lora_bandwidth_k_hz"]) >= 250
    ):
      raise ValueError("LoRa 250/500 kHz requires RF frequency >= 400 MHz")
    if (
      int(values["lora_spreading_factor"]) <= 6
      and int(values["preamble_length"]) < 12
    ):
      raise ValueError("LoRa SF5/SF6 requires preamble length >= 12")

  if packet_type in ("fsk", "msk"):
    if (
      values["generic_address_filtering"] != "disabled"
      and int(values["payload_length_bytes"]) > 254
    ):
      raise ValueError("address filtering requires payload length <= 254")

  if packet_type == "fsk":
    occupied = (
      Decimal(values["generic_frequency_deviation_hz"])
      + Decimal(values["generic_bit_rate_bps"]) / 2
    )
    bandwidth = Decimal(values["fsk_receive_bandwidth_k_hz"]) * 1000
    if occupied > 250000 or 2 * occupied >= bandwidth:
      raise ValueError(
        "FSK deviation/bit rate exceeds modulation or RX bandwidth"
      )

  if (
    values.get("rx_mode") == "duty_cycle"
    and values["rx_duty_cycle_rx_period"] != "until_packet"
  ):
    window = 2 * Decimal(values["rx_duty_cycle_rx_period"]) + Decimal(
      values["rx_duty_cycle_sleep_period"]
    )
    if packet_type == "fsk":
      duration = (
        Decimal(values["preamble_length"])
        + Decimal(values["generic_sync_word_length_bits"])
      ) / Decimal(values["generic_bit_rate_bps"])
    else:
      # RM0461 section 4.5.2: preamble extension and explicit header symbols.
      symbols = Decimal(values["preamble_length"]) + Decimal("4.25")
      if values["lora_header_mode"] == "explicit":
        symbols += 8
      duration = (
        symbols
        * (2 ** int(values["lora_spreading_factor"]))
        / (Decimal(values["lora_bandwidth_k_hz"]) * 1000)
      )

    if duration >= window:
      raise ValueError(
        "RX duty-cycle window must exceed preamble/header duration"
      )


##
# @brief Serialize only the selected complete block, rejecting foreign options.
# @param[in] args Parsed command arguments.
# @return Complete configuration payload.
##
def configuration_payload(args):
  fields = COMMON_FIELDS + CONFIG_FIELDS[args.packet_type]
  selected = {name for name, _, _ in fields}
  all_names = {field[0] for block in CONFIG_FIELDS.values() for field in block}
  for name in all_names - selected:
    if getattr(args, name) is not None:
      raise ValueError(
        f"--{name.replace('_', '-')} is not in this packet block"
      )

  payload = bytearray()
  values = {}
  for field in fields:
    name, _, kind = field
    text = getattr(args, name)
    if text is None:
      raise ValueError(f"missing --{name.replace('_', '-')}")

    try:
      raw = encode_value(text, field)
    except (ValueError, argparse.ArgumentTypeError) as error:
      raise ValueError(f"--{name.replace('_', '-')}: {error}") from error

    payload.extend(raw)
    values[name] = format_value(raw, kind)

  validate_configuration(values)
  return bytes(payload)


##
# @brief Format a complete RX report using the current packet type.
# @param[in] payload Received packet payload.
# @param[in] packet_type Configuration packet type.
# @return Output record; raises ValueError for malformed payloads.
##
def packet_record(payload, packet_type):
  if len(payload) < 3 or len(payload) != payload[0] + 3:
    raise ValueError("RX packet size mismatch")

  record = (
    f"event=rx_packet received_packet_length_bytes={payload[0]}"
    f" rx_packet_data={payload[1:-2].hex()}"
    f" received_packet_rssi_dbm={Decimal(payload[-2]) * Decimal('-0.5'):f}"
  )

  if packet_type == "lora":
    snr = int.from_bytes(payload[-1:], "little", signed=True)
    return (
      record + f" received_packet_snr_db={Decimal(snr) * Decimal('0.25'):f}"
    )
  return record + f" received_packet_status={payload[-1]:02x}"


##
# @brief Follow RX/CAD until an unambiguous report or diagnostic termination.
# @param[in] serial_fd Open device descriptor.
# @param[in] args CLI command and per-query timeout.
##
def follow_operation(serial_fd, args):
  # Report decoding and completion rules depend on the active configuration.
  write_frame(serial_fd, encode_frame(0))
  config = dict(
    field.split("=", 1)
    for field in read_response(serial_fd, 0x8001, args.timeout).split()
  )
  packet_type = config["packet_type"]
  if packet_type not in (
    ("lora", "fsk") if args.command == "rx" else ("lora",)
  ):
    raise OSError(f"{args.command} is unavailable for {packet_type}")

  reader = FrameReader(serial_fd)
  try:
    write_frame(serial_fd, encode_frame(COMMANDS[args.command][0]))
    poll_at = time.monotonic() + POLL_INTERVAL_SECONDS
    query_deadline = None

    while True:
      # Silence triggers a diagnostic query, not operation completion.
      if query_deadline is None and time.monotonic() >= poll_at:
        write_frame(serial_fd, encode_frame(9))
        query_deadline = time.monotonic() + args.timeout

      try:
        response_id, payload = reader.read(
          poll_at if query_deadline is None else query_deadline
        )
      except TimeoutError:
        if query_deadline is not None:
          raise
        continue

      try:
        if response_id == 0x8003:
          if args.command == "cad" and config["cad_exit_mode"] != (
            "receive_on_detection"
          ):
            continue
          record = packet_record(payload, packet_type)
          print(record, flush=True)
          if args.command == "rx" and config["rx_mode"] == "single":
            return
        elif response_id == 0x8004 and args.command == "cad":
          if payload:
            raise ValueError("CAD report must have an empty payload")
          print("event=cad_detected", flush=True)
          if config["cad_exit_mode"] == "stop_on_detection":
            return
        elif response_id == 0x8002 and query_deadline is not None:
          response_record(response_id, payload)

          query_deadline = None
          poll_at = time.monotonic() + POLL_INTERVAL_SECONDS

          if int.from_bytes(payload[1:], "little"):
            raise OSError(f"recorded radio errors: {payload[1:][::-1].hex()}")
          if ACTIVE_OPERATIONS[payload[0]] != args.command:
            return
      except ValueError as error:
        print(f"Discarded frame: {error}", file=sys.stderr)
  except KeyboardInterrupt:
    # Attempt STOP even if interruption occurred while queuing the start frame.
    write_frame(serial_fd, encode_frame(8))
    raise


##
# @brief Parse a strictly positive finite host response timeout.
# @param[in] text Command-line timeout value, in seconds.
# @return Validated float; raises ArgumentTypeError for invalid values.
##
def positive_timeout(text):
  try:
    value = float(text)
  except ValueError as error:
    raise argparse.ArgumentTypeError("timeout must be a number") from error

  if not math.isfinite(value) or value <= 0:
    raise argparse.ArgumentTypeError("timeout must be finite and positive")
  return value


##
# @brief Parse an exactly representable image-calibration frequency in Hz.
# @param[in] text Physical frequency, without a unit suffix.
# @return Encoded byte value; raises ArgumentTypeError for invalid values.
##
def calibration_frequency(text):
  try:
    value = Decimal(text)
  except ArithmeticError as error:
    raise argparse.ArgumentTypeError("frequency must be a number") from error

  if (
    not value.is_finite()
    or not 152000000 <= value <= 960000000
    or value % 4000000 != 0
  ):
    raise argparse.ArgumentTypeError(
      "frequency must be 152000000..960000000 Hz in steps of 4000000 Hz"
    )

  return int(value / 4000000)


##
# @brief Parse up to 255 bytes of contiguous lowercase hexadecimal packet data.
# @param[in] text Packet bytes with no prefix or separators; may be empty.
# @return Payload bytes; raises ArgumentTypeError for invalid input.
##
def packet_data(text):
  if (
    len(text) > 510
    or len(text) % 2
    or any(value not in "0123456789abcdef" for value in text)
  ):
    raise argparse.ArgumentTypeError(
      "packet data must be 0..255 bytes of contiguous lowercase hexadecimal"
    )
  return bytes.fromhex(text)


##
# @brief Parse arguments before device access; route even help to stderr.
# @param[in] argv Argument sequence without script name, or None for sys.argv.
# @return Parsed arguments; argparse exits on help or invalid input.
##
def parse_arguments(argv):
  parser = argparse.ArgumentParser(
    description="STM32WLE5 RF dongle CLI (ten commands).",
    allow_abbrev=False,
  )
  parser.add_argument("--device", required=True, help="Virtual COM Port path")
  parser.set_defaults(payload=b"")

  commands = parser.add_subparsers(dest="command", required=True)
  for name, (_, response_id) in COMMANDS.items():
    command = commands.add_parser(name, allow_abbrev=False)
    if name == "set-config":
      descriptors = {field[0]: field for field in COMMON_FIELDS}
      for block in CONFIG_FIELDS.values():
        descriptors.update({field[0]: field for field in block})

      for field_name, _, kind in descriptors.values():
        help_text = "physical value, exactly representable"
        if isinstance(kind, dict):
          help_text = "one of: " + ", ".join(map(str, kind.values()))
        elif kind == "bytes":
          help_text = "exactly 8 bytes of contiguous lowercase hexadecimal"
        command.add_argument(
          "--" + field_name.replace("_", "-"), help=help_text
        )
    elif name == "calibrate":
      for boundary in ("lower", "upper"):
        command.add_argument(
          f"--image-calibration-{boundary}-frequency-hz",
          type=calibration_frequency,
          required=True,
          metavar="HZ",
          help=f"{boundary} frequency, 152..960 MHz in 4 MHz steps",
        )
    elif name == "tx-packet":
      command.add_argument(
        "--tx-packet-data",
        type=packet_data,
        required=True,
        dest="payload",
        metavar="HEX",
        help="0..255 bytes; for BPSK supply the complete frame, including CRC",
      )

    if response_id is not None:
      command.add_argument(
        "--timeout",
        type=positive_timeout,
        default=DEFAULT_TIMEOUT_SECONDS,
        metavar="SECONDS",
        help="per-query response wait in seconds (default: 2)",
      )

  with contextlib.redirect_stdout(sys.stderr):
    args = parser.parse_args(argv)

    if args.command == "set-config":
      if args.packet_type not in CONFIG_FIELDS:
        parser.error("--packet-type must be lora, fsk, msk or bpsk")
      try:
        args.payload = configuration_payload(args)
      except ValueError as error:
        parser.error(str(error))

    if args.command == "calibrate":
      lower = args.image_calibration_lower_frequency_hz
      upper = args.image_calibration_upper_frequency_hz
      if lower > upper:
        parser.error("lower calibration frequency must not exceed upper")
      args.payload = bytes((lower, upper))

    return args


##
# @brief Execute one command; keep errors and help off the record stream.
# @param[in] argv Argument sequence, or None to use the process arguments.
# @return Zero on success/help, 1 on I/O failure, 2 on arguments, 130 on Ctrl-C.
##
def main(argv=None):
  try:
    args = parse_arguments(argv)
  except SystemExit as error:
    return error.code

  try:
    serial_fd = open_serial(args.device)
    try:
      request_id, response_id = COMMANDS[args.command]
      if args.command in ("rx", "cad"):
        follow_operation(serial_fd, args)
      else:
        write_frame(serial_fd, encode_frame(request_id, args.payload))
        if response_id is None:
          record = f"event=request_sent command={args.command}"
        else:
          record = read_response(serial_fd, response_id, args.timeout)
        print(record, flush=True)
    finally:
      os.close(serial_fd)
    return 0
  except KeyboardInterrupt:
    print("Interrupted", file=sys.stderr)
    return 130
  except (OSError, termios.error) as error:
    print(f"Error: {error}", file=sys.stderr)
    return 1


# ==============================================================================
# SCRIPT ENTRY POINT
# ==============================================================================

if __name__ == "__main__":
  sys.exit(main())
