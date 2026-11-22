# ==============================================================================
# @brief Product stack watermark integration test (SW_INTEGRATION_TEST_10).
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
import contextlib
import json
import os
import re
import selectors
import shlex
import subprocess
import tempfile
import time
from collections import deque
from pathlib import Path

# ------------------------------------------------------------------------------
# External library imports
# ------------------------------------------------------------------------------
import pytest

# ==============================================================================
# CONSTANTS
# ==============================================================================

##
# @brief Test acceptance policy, not an architecture requirement.
##
MIN_FREE_PERCENT = 30
##
# @brief Repetitions of both RF profiles without intermediate debugger halts.
##
CYCLES = 3
##
# @brief GDB response and remote command deadline budget in seconds.
##
COMMAND_TIMEOUT = 15
##
# @brief Watermark byte; identical writes remain undetectable.
##
PATTERN = 0xA5
##
# @brief Workspace root for the public CLI and existing debug launcher.
##
ROOT = Path(__file__).resolve().parents[5]

##
# @brief Shared public CLI configuration for finite, non-transmitting RX.
##
COMMON = {
  "rf_frequency_hz": "868000000",
  "payload_length_bytes": "32",
  "tx_output_power_dbm": "-9",
  "preamble_length": "8",
  "operation_timeout": "disabled",
  "tx_ramp_time_us": "40",
  "rx_mode": "single",
  "rx_timeout_control": "0.0625",
  "rx_duty_cycle_rx_period": "until_packet",
  "rx_duty_cycle_sleep_period": "0.03125",
}
##
# @brief Packet-type-specific fields completing each public CLI configuration.
##
PROFILES = {
  "lora": {
    "lora_spreading_factor": "7",
    "lora_bandwidth_k_hz": "125",
    "lora_coding_rate": "4/5",
    "lora_low_data_rate_optimize": "disabled",
    "lora_header_mode": "explicit",
    "lora_payload_crc_enabled": "enabled",
    "lora_invert_iq": "standard",
    "lora_synchronization_word": "private",
    "lora_symbol_timeout": "0",
    "cad_scan_symbol_count": "4",
    "cad_detection_peak_threshold": "22",
    "cad_detection_minimum_threshold": "10",
    "cad_exit_mode": "stop_on_detection",
    "cad_timeout": "0.0625",
  },
  "fsk": {
    "generic_bit_rate_bps": "1000",
    "generic_frequency_deviation_hz": "3906.25",
    "generic_pulse_shape": "none",
    "fsk_receive_bandwidth_k_hz": "467",
    "generic_payload_length_mode": "variable",
    "generic_preamble_detection_length_bits": "8",
    "generic_sync_word_length_bits": "16",
    "generic_sync_word": "123456789abcdef0",
    "generic_address_filtering": "disabled",
    "generic_node_address": "18",
    "generic_broadcast_address": "52",
    "generic_crc_type": "crc16",
    "generic_crc_initial_value": "65535",
    "generic_crc_polynomial": "4129",
    "generic_whitening_enabled": "enabled",
    "generic_whitening_initial_value": "511",
  },
}

# ==============================================================================
# CLASSES
# ==============================================================================


##
# @brief Keep one asynchronous GDB/MI connection throughout the measurement.
##
class Gdb:
  ##
  # @brief Start GDB without init files or embedded Python dependencies.
  # @param[in] elf Product ELF path.
  ##
  def __init__(self, elf):
    self.process = subprocess.Popen(
      ["arm-none-eabi-gdb", "-nx", "-q", "--interpreter=mi2", str(elf)],
      stdin=subprocess.PIPE,
      stdout=subprocess.PIPE,
      stderr=subprocess.STDOUT,
    )
    self.selector = selectors.DefaultSelector()
    self.selector.register(self.process.stdout, selectors.EVENT_READ)
    self.pending = b""
    self.token = 0
    self.stopped = False
    self.transcript = deque(maxlen=12)

  ##
  # @brief Read one MI line within an absolute deadline.
  # @param[in] deadline Monotonic deadline.
  # @return Decoded MI line.
  ##
  def line(self, deadline):
    while b"\n" not in self.pending:
      remaining = deadline - time.monotonic()
      assert remaining > 0 and self.selector.select(remaining), "GDB timeout"
      chunk = os.read(self.process.stdout.fileno(), 4096)
      assert chunk, "GDB exited unexpectedly"
      self.pending += chunk

    line, self.pending = self.pending.split(b"\n", 1)
    text = line.decode(errors="replace")
    self.transcript.append(text[-300:])
    if text.startswith("*stopped"):
      self.stopped = True
    return text

  ##
  # @brief Send a tokenized MI command and reject GDB errors.
  # @param[in] command MI command text.
  # @return All output through the command result.
  ##
  def command(self, command):
    self.token += 1
    token = str(self.token)
    self.process.stdin.write(f"{token}{command}\n".encode())
    self.process.stdin.flush()
    lines = []
    deadline = time.monotonic() + COMMAND_TIMEOUT
    while True:
      line = self.line(deadline)
      lines.append(line)
      if line.startswith(token + "^"):
        assert not line.startswith(token + "^error"), line
        return "\n".join(lines)

  ##
  # @brief Execute a GDB console command through MI.
  # @param[in] command Console command text.
  # @return Command output.
  ##
  def console(self, command):
    return self.command("-interpreter-exec console " + json.dumps(command))

  ##
  # @brief Evaluate an integer expression without parsing target structures.
  # @param[in] expression GDB expression.
  # @return Integer value.
  ##
  def value(self, expression):
    output = self.command(
      "-data-evaluate-expression "
      + json.dumps(f"(unsigned long)({expression})")
    )
    match = re.search(r'value="(0x[0-9a-fA-F]+|[0-9]+)"', output)
    assert match, output
    return int(match[1], 0)

  ##
  # @brief Continue asynchronously, optionally awaiting a breakpoint.
  # @param[in] wait True to wait for the next stop.
  ##
  def resume(self, wait=False):
    self.stopped = False
    self.command("-exec-continue")
    if wait:
      self.wait_stop()

  ##
  # @brief Await a consistent stopped target state.
  ##
  def wait_stop(self):
    deadline = time.monotonic() + COMMAND_TIMEOUT
    while not self.stopped:
      self.line(deadline)

  ##
  # @brief Halt the running target for the final snapshot.
  ##
  def halt(self):
    self.command("-exec-interrupt")
    self.wait_stop()

  ##
  # @brief Remove breakpoints, detach and reap only this GDB process.
  ##
  def close(self):
    try:
      if not self.stopped:
        self.halt()
      self.command("-break-delete")
      self.command("-target-detach")
      self.command("-gdb-exit")
    finally:
      if self.process.poll() is None:
        self.process.terminate()
      self.process.wait(timeout=COMMAND_TIMEOUT)
      self.selector.close()


# ==============================================================================
# FUNCTIONS
# ==============================================================================


##
# @brief Report a phase before blocking and completion only after its checks.
# @param[in] label Short phase or operation description.
##
@contextlib.contextmanager
def progress(label):
  print(f"{label} ...", flush=True)
  try:
    yield
  except (Exception, pytest.fail.Exception) as error:
    print(f"{label}: FAIL: {str(error)[-2000:]}", flush=True)
    if isinstance(
      error, (subprocess.CalledProcessError, subprocess.TimeoutExpired)
    ):
      print(
        f"Last command output: {error.stdout}\n{error.stderr}"[-2000:],
        flush=True,
      )
    raise
  else:
    print(f"{label}: OK", flush=True)


##
# @brief Report measured margins before applying the existing acceptance check.
# @param[in] capacity Actual stack capacity in bytes.
# @param[in] free Measured free bytes.
# @return Free percentage used by the acceptance check.
##
def summary(capacity, free):
  percent = 100 * free / capacity
  used = capacity - free
  minimum = (capacity * MIN_FREE_PERCENT + 99) // 100
  result = "PASS" if percent >= MIN_FREE_PERCENT else "FAIL"
  print(
    f"\nStack watermark summary\n"
    f"  Capacity: {capacity} B\n"
    f"  Used: {used} B ({100 * used / capacity:.2f}%)\n"
    f"  Free: {free} B ({percent:.2f}%)\n"
    f"  Minimum free: {MIN_FREE_PERCENT}% (ceil {minimum} B)\n"
    f"  Result: {result}\n",
    flush=True,
  )
  return percent


##
# @brief Read required configuration, failing before any hardware access.
# @param[in] name Environment variable name.
# @return Nonempty configured value.
##
def required(name):
  value = os.getenv(name)
  assert value, f"Missing required environment variable: {name}"
  return value


##
# @brief Run an owned remote command with bounded transport and execution time.
# @param[in] peer SSH user and host.
# @param[in] command Remote command arguments.
# @param[in] source Optional standard input.
# @return Completed command standard output.
##
def remote(peer, command, source=None):
  result = subprocess.run(
    [
      "ssh",
      "-o",
      "BatchMode=yes",
      "-o",
      "ConnectTimeout=10",
      peer,
      shlex.join(command),
    ],
    input=source,
    text=True,
    capture_output=True,
    timeout=COMMAND_TIMEOUT,
  )
  assert result.returncode == 0, (result.stdout + result.stderr)[-2000:]
  return result.stdout


##
# @brief Select the product FT230X, never the separate logging UART.
# @param[in] peer SSH user and host.
# @return First product adapter by-id path in sorted order.
##
def product_vcp(peer):
  devices = json.loads(
    remote(
      peer,
      [
        "python3",
        "-c",
        (
          "import glob,json; print(json.dumps(sorted(glob.glob("
          "'/dev/serial/by-id/usb-FTDI_FT230X_Basic_UART_*-if00-port0'))))"
        ),
      ],
    )
  )
  assert devices, (
    "Product VCP not found: no FT230X by-id path on the Raspberry Pi"
  )
  device = devices[0]
  print(f"Product VCP: {device}", flush=True)
  return device


##
# @brief Hold the cooperative target lock and reject a busy product VCP.
# @param[in] peer SSH user and host.
# @param[in] device Explicit product VCP.
##
@contextlib.contextmanager
def target_access(peer, device):
  # flock is held until this SSH session closes; no server process is owned.
  script = f"""
set -eu
test -c {shlex.quote(device)} || {{ echo 'Product VCP missing'; exit 1; }}
if fuser {shlex.quote(device)} >/dev/null 2>&1; then
  echo 'Product VCP busy'; exit 1
fi
echo READY
read -r done
"""
  process = subprocess.Popen(
    [
      "ssh",
      "-o",
      "BatchMode=yes",
      "-o",
      "ConnectTimeout=10",
      peer,
      shlex.join(
        ["flock", "-n", "/tmp/ew-hil-target.lock", "sh", "-c", script]
      ),
    ],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.STDOUT,
  )
  try:
    with selectors.DefaultSelector() as selector:
      selector.register(process.stdout, selectors.EVENT_READ)
      assert selector.select(COMMAND_TIMEOUT), "Remote hardware lock timeout"
      message = process.stdout.readline().decode().strip()
      assert message == "READY", message or "Remote hardware lock unavailable"
    yield
  finally:
    # EOF releases flock, also on setup failure; leave the server alone.
    process.stdin.close()
    try:
      process.wait(timeout=COMMAND_TIMEOUT)
    except subprocess.TimeoutExpired:
      process.terminate()
      process.wait(timeout=COMMAND_TIMEOUT)


##
# @brief Refuse to connect to a server with an established debugger connection.
# @param[in] peer SSH user and host.
# @param[in] port Configured GDB port.
##
def require_idle_debugger(peer, port):
  connections = remote(
    peer, ["ss", "-H", "-tn", "state", "established", f"sport = :{port}"]
  )
  assert not connections.strip(), (
    f"GDB port {port} has an active client; finish that debug session "
    "before running the test (no connection or reset attempted)"
  )


##
# @brief Invoke the unmodified public CLI on its POSIX VCP host.
# @param[in] peer SSH user and host.
# @param[in] device Product VCP path.
# @param[in] args Public CLI arguments.
# @return Parsed public key=value records, without protocol knowledge.
##
def cli(peer, device, *args):
  output = remote(
    peer,
    [
      "timeout",
      "--signal=INT",
      "--kill-after=2",
      "10",
      "python3",
      "-",
      "--device",
      device,
      *args,
    ],
    (ROOT / "sw/cli/cli.py").read_text(),
  )
  try:
    return [
      dict(field.split("=", 1) for field in line.split())
      for line in output.splitlines()
    ]
  except ValueError as error:
    raise AssertionError(f"CLI {args[0]} reply: {output[-2000:]}") from error


##
# @brief Await public RF idle with bounded diagnostic queries.
# @param[in] peer SSH user and host.
# @param[in] device Product VCP path.
##
def idle(peer, device):
  deadline = time.monotonic() + COMMAND_TIMEOUT
  while time.monotonic() < deadline:
    records = cli(peer, device, "get-diagnostics", "--timeout", "2")
    assert len(records) == 1 and records[0]["event"] == "diagnostics", records
    assert records[0]["radio_errors"] == "0000", records
    if records[0]["active_rf_operation"] == "idle":
      return
  pytest.fail(f"Product did not reach public RF idle; last reply: {records}")


##
# @brief Execute bounded public configuration, RX, STOP and calibration cycles.
# @param[in] peer SSH user and host.
# @param[in] device Product VCP path.
##
def workload(peer, device):
  with progress("Workload initial RF idle"):
    idle(peer, device)
  for cycle in range(CYCLES):
    for packet_type, profile in PROFILES.items():
      print(
        f"\nWorkload cycle {cycle + 1}/{CYCLES}: {packet_type.upper()}",
        flush=True,
      )
      values = COMMON | profile | {"packet_type": packet_type}
      options = [
        item
        for key, value in values.items()
        for item in ("--" + key.replace("_", "-"), value)
      ]
      with progress("  Configuration verified"):
        records = cli(peer, device, "set-config", *options)
        assert records == [
          {"event": "request_sent", "command": "set-config"}
        ], records
        records = cli(peer, device, "get-config", "--timeout", "2")
        assert records == [{"event": "config"} | values], records
      # Finite RX lets the public one-shot CLI own all polling and decoding.
      with progress("  RX and status idle"):
        reports = cli(peer, device, "rx", "--timeout", "2")
        assert all(record["event"] == "rx_packet" for record in reports), (
          reports
        )
        idle(peer, device)
      with progress("  STOP and idle"):
        cli(peer, device, "stop")
        idle(peer, device)
      with progress("  Calibration and idle"):
        cli(
          peer,
          device,
          "calibrate",
          "--image-calibration-lower-frequency-hz",
          "860000000",
          "--image-calibration-upper-frequency-hz",
          "880000000",
        )
        idle(peer, device)


##
# @brief Conservatively scan the watermark and currently reserved stack.
# @param[in] data Complete stack snapshot, bottom first.
# @param[in] paint_size Initially painted prefix size.
# @param[in] reserved_offset Current MSP offset from stack bottom.
# @return Free bytes below every observed overwrite/reservation.
##
def free_bytes(data, paint_size, reserved_offset):
  assert 0 <= paint_size <= len(data)
  assert 0 <= reserved_offset <= len(data)
  # MSP can reserve unwritten bytes, so matching paint alone is insufficient.
  limit = min(paint_size, reserved_offset)
  return next(
    (index for index in range(limit) if data[index] != PATTERN), limit
  )


##
# @brief Measure actual product stack use under public, non-transmitting load.
##
def test_stack_watermark():
  peer = required("RPI_USER") + "@" + required("RPI_HOST")
  host = required("RPI_HOST")
  port = int(required("GDB_PORT"))
  assert 0 < port < 65536
  elf = Path(required("STACK_PRODUCT_ELF")).resolve()
  assert elf.is_file(), (
    f"Product ELF missing; build the selected preset first: {elf}"
  )

  with progress("Prepare product VCP"):
    device = product_vcp(peer)

  with (
    target_access(peer, device),
    tempfile.TemporaryDirectory(
      prefix="stack-watermark-", dir="/tmp"
    ) as temporary,
  ):
    with progress("Prepare server (idle debugger required)"):
      require_idle_debugger(peer, port)
      subprocess.run(
        [str(ROOT / ".vscode/tasks/run_target_gdb_server.sh")],
        check=True,
        capture_output=True,
        text=True,
        timeout=COMMAND_TIMEOUT + float(required("NETWORK_LATENCY_TIMEOUT_S")),
      )
      require_idle_debugger(peer, port)

    with contextlib.closing(Gdb(elf)) as gdb:
      measure(gdb, host, port, peer, device, temporary)


##
# @brief Paint and measure the product through the test's own GDB connection.
# @param[in] gdb Owned GDB/MI connection.
# @param[in] host Remote GDB server host.
# @param[in] port Remote GDB server port.
# @param[in] peer SSH user and host for public CLI access.
# @param[in] device Product VCP path.
# @param[in] temporary Directory for watermark and snapshot files.
##
def measure(gdb, host, port, peer, device, temporary):
  running = False
  safe_idle = False
  try:
    with progress("Prepare connect"):
      gdb.command("-gdb-set mi-async on")
      gdb.command(f"-target-select remote {host}:{port}")
      gdb.stopped = True
    with progress("Prepare program and verify"):
      gdb.console("monitor reset")
      gdb.console("monitor halt")
      gdb.console("load")
      comparison = gdb.console("compare-sections")
      assert "MIS-MATCHED" not in comparison and "warning:" not in comparison, (
        comparison
      )

    print("Prepare watermark (main entry, paint and readback) ...", flush=True)
    gdb.console("monitor reset")
    gdb.console("monitor halt")
    # Break before the prologue to avoid painting main's reserved frame.
    entry = gdb.value("(unsigned long)&main") & ~1
    gdb.command(f"-break-insert -h *{entry}")
    gdb.resume(wait=True)
    bottom = gdb.value("(unsigned long)&_stack_bottom")
    top = gdb.value("(unsigned long)&_stack_top")
    msp = gdb.value("$msp")
    assert gdb.value("$pc") & ~1 == entry, "Not stopped at main entry"
    assert gdb.value("$xpsr") & (1 << 24), "Not in Thumb state"
    assert gdb.value("$xpsr") & 0x1FF == 0, "Not in thread mode"
    assert gdb.value("$control") & 2 == 0, "Thread uses PSP instead of MSP"
    assert bottom < msp <= top and all(x % 8 == 0 for x in (bottom, msp, top))

    pattern = Path(temporary) / "pattern.bin"
    snapshot = Path(temporary) / "stack.bin"
    pattern.write_bytes(bytes([PATTERN]) * (msp - bottom))
    gdb.console(f"restore {pattern} binary {bottom}")
    gdb.console(f"dump binary memory {snapshot} {bottom} {msp}")
    assert snapshot.read_bytes() == pattern.read_bytes(), (
      "Paint readback failed"
    )
    gdb.command("-break-delete")
    gdb.resume()
    running = True
    print("Watermark prepared: OK", flush=True)

    workload(peer, device)
    with progress("\nFinal STOP and RF idle"):
      cli(peer, device, "stop")
      idle(peer, device)
      safe_idle = True

    print("Read stack (halt and snapshot) ...", flush=True)
    gdb.halt()
    final_msp = gdb.value("$msp")
    assert bottom <= final_msp <= top and final_msp % 4 == 0
    gdb.console(f"dump binary memory {snapshot} {bottom} {top}")
    data = snapshot.read_bytes()
    capacity = top - bottom
    assert len(data) == capacity
    free = free_bytes(data, msp - bottom, final_msp - bottom)

    print("Read stack: OK", flush=True)
    percent = summary(capacity, free)
    assert percent >= MIN_FREE_PERCENT, "Insufficient measured stack margin"
  except (Exception, pytest.fail.Exception) as error:
    print(f"Stack measurement: FAIL: {str(error)[-2000:]}", flush=True)
    print("Last GDB records:\n" + "\n".join(gdb.transcript), flush=True)
    raise
  finally:
    if running and not safe_idle:
      if gdb.stopped:
        with contextlib.suppress(Exception):
          gdb.resume()
      try:
        cli(peer, device, "stop")
        idle(peer, device)
      except Exception as error:
        print(
          f"STOP cleanup failed; resetting owned product: {error}", flush=True
        )
        if not gdb.stopped:
          gdb.halt()
        gdb.console("monitor reset")
        gdb.console("monitor halt")
        gdb.resume()
        idle(peer, device)
