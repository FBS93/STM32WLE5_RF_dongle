#!/usr/bin/env python3

# ==============================================================================
# @brief Launch and inspect detached asynchronous jobs.
#
# Each job runs without a shell and stores its request, status and combined log
# under the workspace build directory. State files are published atomically.
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
import fcntl
import json
import os
import re
import secrets
import shutil
import signal
import subprocess
import sys
import tempfile
import time
from datetime import datetime
from datetime import timezone
from pathlib import Path

# ------------------------------------------------------------------------------
# External library imports
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Project-specific imports
# ------------------------------------------------------------------------------

# ==============================================================================
# CONSTANTS
# ==============================================================================

##
# @brief Tool name used for the source and build directories.
##
TOOL_NAME = "async_job"

##
# @brief Version of the persisted request and status formats.
##
SCHEMA_VERSION = 1

##
# @brief Number of random bytes encoded in a generated job identifier.
##
JOB_ID_RANDOM_BYTES = 12

##
# @brief Regular expression accepted for externally supplied job identifiers.
##
JOB_ID_PATTERN = re.compile(r"^[0-9a-f]{24}$")

##
# @brief Default maximum number of trailing log bytes returned by @c log.
##
DEFAULT_LOG_BYTES = 65536

##
# @brief Seconds allowed for graceful process-group termination.
##
CANCEL_TERMINATE_TIMEOUT_SECONDS = 1.0

##
# @brief Seconds allowed after escalation for process-group termination.
##
CANCEL_KILL_TIMEOUT_SECONDS = 1.0

##
# @brief Interval used for bounded cancellation state checks.
##
CANCEL_CHECK_INTERVAL_SECONDS = 0.01

##
# @brief Lifecycle states returned by the active-job registry.
##
ACTIVE_STATES = frozenset(("queued", "running"))

##
# @brief Lifecycle states retained as terminal job artifacts.
##
TERMINAL_STATES = frozenset(("succeeded", "failed", "interrupted", "cancelled"))

# ==============================================================================
# CLASSES
# ==============================================================================


class AsyncJobError(Exception):
  ##
  # @brief Represent an invalid request or inaccessible job artifact.
  ##

  pass


# ==============================================================================
# FUNCTIONS
# ==============================================================================


##
# @brief Parse command-line arguments.
#
# @param[in] arguments Optional argument list excluding the executable name.
# @return Parsed command-line arguments.
##
def parse_args(arguments=None):
  if arguments is None:
    arguments = sys.argv[1:]

  if arguments and arguments[0] == "_worker":
    worker_parser = argparse.ArgumentParser(add_help=False)
    worker_parser.set_defaults(action="_worker")
    worker_parser.add_argument("job_id")
    return worker_parser.parse_args(arguments[1:])

  parser = argparse.ArgumentParser(
    description="Launch and inspect detached asynchronous jobs."
  )
  subparsers = parser.add_subparsers(dest="action", required=True)

  start_parser = subparsers.add_parser(
    "start",
    help="Start one detached job and print its job ID.",
  )
  start_parser.add_argument(
    "--cwd",
    default=Path("."),
    type=Path,
    help="Command working directory within the workspace (default: workspace).",
  )
  start_parser.add_argument(
    "executable",
    help="Executable path or name.",
  )
  start_parser.add_argument(
    "arguments",
    nargs=argparse.REMAINDER,
    help="Arguments passed verbatim to the executable.",
  )

  status_parser = subparsers.add_parser(
    "status",
    help="Print the current job status as JSON without waiting.",
  )
  status_parser.add_argument("job_id", help="Job identifier returned by start.")

  subparsers.add_parser(
    "list",
    help="List active jobs and their current states as JSON.",
  )

  cancel_parser = subparsers.add_parser(
    "cancel",
    help="Cancel one active job and its process group.",
  )
  cancel_parser.add_argument("job_id", help="Active job identifier.")

  clean_parser = subparsers.add_parser(
    "clean",
    help="Remove one terminal job's artifacts.",
  )
  clean_parser.add_argument("job_id", help="Terminal job identifier.")

  log_parser = subparsers.add_parser(
    "log",
    help="Print a snapshot of the combined job log without waiting.",
  )
  log_parser.add_argument("job_id", help="Job identifier returned by start.")
  log_selection = log_parser.add_mutually_exclusive_group()
  log_selection.add_argument(
    "--bytes",
    default=DEFAULT_LOG_BYTES,
    type=positive_integer,
    help=f"Return at most the trailing N bytes (default: {DEFAULT_LOG_BYTES}).",
  )
  log_selection.add_argument(
    "--all",
    action="store_true",
    help="Return the complete log instead of a bounded tail.",
  )

  return parser.parse_args(arguments)


##
# @brief Parse a strictly positive integer command-line value.
#
# @param[in] value Text supplied by the command-line parser.
# @return Parsed positive integer.
##
def positive_integer(value):
  parsed_value = int(value)
  if parsed_value <= 0:
    raise argparse.ArgumentTypeError("value must be greater than zero")
  return parsed_value


##
# @brief Return the fixed workspace root containing this tool.
#
# @return Resolved workspace root path.
##
def workspace_root():
  return Path(__file__).resolve().parents[2]


##
# @brief Return the build directory reserved for this tool.
#
# @return Absolute tool build directory path.
##
def tool_build_directory():
  return workspace_root() / "build" / TOOL_NAME


##
# @brief Return a UTC timestamp in an ISO 8601 representation.
#
# @return Current timestamp with millisecond precision.
##
def utc_timestamp():
  return (
    datetime.now(timezone.utc)
    .isoformat(timespec="milliseconds")
    .replace("+00:00", "Z")
  )


##
# @brief Ensure a directory exists and is not a symbolic link.
#
# @param[in] directory_path Directory to create or validate.
##
def ensure_directory(directory_path):
  try:
    directory_path.mkdir(mode=0o700, parents=True, exist_ok=True)
  except OSError as error:
    raise AsyncJobError(
      f"cannot create directory {directory_path}: {error}"
    ) from error

  if directory_path.is_symlink() or not directory_path.is_dir():
    raise AsyncJobError(f"unsafe directory path: {directory_path}")


##
# @brief Atomically publish one JSON object with private permissions.
#
# @param[in] file_path Destination JSON path.
# @param[in] payload JSON-compatible object to publish.
##
def write_json_atomic(file_path, payload):
  temporary_path = None
  try:
    descriptor, temporary_name = tempfile.mkstemp(
      dir=file_path.parent,
      prefix=f".{file_path.name}.",
      suffix=".tmp",
    )
    temporary_path = Path(temporary_name)
    with os.fdopen(descriptor, "w", encoding="utf-8") as file_handle:
      json.dump(payload, file_handle, indent=2, sort_keys=True)
      file_handle.write("\n")
      file_handle.flush()
      os.fsync(file_handle.fileno())
    os.replace(temporary_path, file_path)
  except OSError as error:
    if temporary_path is not None:
      temporary_path.unlink(missing_ok=True)
    raise AsyncJobError(f"cannot write {file_path}: {error}") from error


##
# @brief Read one regular, non-symbolic-link JSON file.
#
# @param[in] file_path JSON file to read.
# @return Parsed JSON object.
##
def read_json(file_path):
  if file_path.is_symlink() or not file_path.is_file():
    raise AsyncJobError(f"missing or unsafe file: {file_path}")

  try:
    with file_path.open("r", encoding="utf-8") as file_handle:
      return json.load(file_handle)
  except (OSError, json.JSONDecodeError) as error:
    raise AsyncJobError(f"cannot read {file_path}: {error}") from error


##
# @brief Validate and resolve a job directory.
#
# @param[in] job_id Candidate job identifier.
# @return Existing private job directory.
##
def existing_job_directory(job_id):
  if JOB_ID_PATTERN.fullmatch(job_id) is None:
    raise AsyncJobError(f"invalid job ID: {job_id}")

  job_directory = tool_build_directory() / job_id
  if job_directory.is_symlink() or not job_directory.is_dir():
    raise AsyncJobError(f"unknown job ID: {job_id}")
  return job_directory


##
# @brief Create a collision-resistant private job directory.
#
# @return Tuple containing the generated identifier and job directory.
##
def create_job_directory():
  ensure_directory(workspace_root() / "build")
  ensure_directory(tool_build_directory())

  for _attempt in range(100):
    job_id = secrets.token_hex(JOB_ID_RANDOM_BYTES)
    job_directory = tool_build_directory() / job_id
    try:
      job_directory.mkdir(mode=0o700)
      return job_id, job_directory
    except FileExistsError:
      continue
    except OSError as error:
      raise AsyncJobError(
        f"cannot create job directory {job_directory}: {error}"
      ) from error

  raise AsyncJobError("cannot allocate a unique job ID")


##
# @brief Resolve and validate a command working directory.
#
# @param[in] requested_cwd Absolute or workspace-relative requested path.
# @return Existing resolved directory contained by the workspace.
##
def resolve_command_cwd(requested_cwd):
  root = workspace_root()
  candidate = requested_cwd
  if not candidate.is_absolute():
    candidate = root / candidate

  try:
    resolved_cwd = candidate.resolve(strict=True)
    resolved_cwd.relative_to(root)
  except (OSError, ValueError) as error:
    raise AsyncJobError(
      f"cwd must be an existing directory within {root}: {requested_cwd}"
    ) from error

  if not resolved_cwd.is_dir():
    raise AsyncJobError(f"cwd is not a directory: {requested_cwd}")
  return resolved_cwd


##
# @brief Create the initial persisted state for one job.
#
# @param[in] job_id Generated job identifier.
# @param[in] queued_at Time at which the request was persisted.
# @return Initial status object.
##
def queued_status(job_id, queued_at):
  return {
    "error": None,
    "exit_code": None,
    "finished_at": None,
    "job_id": job_id,
    "pid": None,
    "queued_at": queued_at,
    "schema_version": SCHEMA_VERSION,
    "started_at": None,
    "state": "queued",
    "worker_pid": None,
  }


##
# @brief Change one active status object to terminal cancelled state.
#
# @param[in] status Current active status object.
# @return Cancelled status object.
##
def cancelled_status(status):
  cancelled = dict(status)
  cancelled.update(
    {
      "error": "job cancelled by user",
      "finished_at": utc_timestamp(),
      "state": "cancelled",
    }
  )
  return cancelled


##
# @brief Create an empty private combined log file.
#
# @param[in] log_path Log path to create exclusively.
##
def create_log(log_path):
  try:
    descriptor = os.open(
      log_path,
      os.O_CREAT | os.O_EXCL | os.O_WRONLY,
      0o600,
    )
    os.close(descriptor)
  except OSError as error:
    raise AsyncJobError(f"cannot create {log_path}: {error}") from error


##
# @brief Persist and launch one detached asynchronous job.
#
# @param[in] executable Executable path or name.
# @param[in] arguments Arguments passed verbatim to the executable.
# @param[in] requested_cwd Requested workspace-contained working directory.
# @return Generated job identifier.
##
def start_job(executable, arguments, requested_cwd):
  command_cwd = resolve_command_cwd(requested_cwd)
  job_id, job_directory = create_job_directory()
  request_path = job_directory / "request.json"
  status_path = job_directory / "status.json"
  log_path = job_directory / "job.log"
  queued_at = utc_timestamp()

  request = {
    "command": [executable, *arguments],
    "cwd": str(command_cwd),
    "job_id": job_id,
    "requested_at": queued_at,
    "schema_version": SCHEMA_VERSION,
  }
  write_json_atomic(request_path, request)
  write_json_atomic(status_path, queued_status(job_id, queued_at))
  create_log(log_path)

  worker_command = [
    sys.executable,
    str(Path(__file__).resolve()),
    "_worker",
    job_id,
  ]
  try:
    with log_path.open("ab", buffering=0) as log_handle:
      subprocess.Popen(
        worker_command,
        cwd=workspace_root(),
        stdin=subprocess.DEVNULL,
        stdout=log_handle,
        stderr=log_handle,
        shell=False,
        start_new_session=True,
        close_fds=True,
      )
  except OSError as error:
    status = queued_status(job_id, queued_at)
    status.update(
      {
        "error": f"worker launch failed: {error}",
        "finished_at": utc_timestamp(),
        "state": "failed",
      }
    )
    write_json_atomic(status_path, status)
    raise AsyncJobError(
      f"cannot launch worker for job {job_id}: {error}"
    ) from error

  return job_id


##
# @brief Acquire a non-blocking exclusive lock on one job directory.
#
# The lock has no filesystem sidecar and remains held until its descriptor is
# closed. It prevents accidental duplicate workers for one persisted request.
#
# @param[in] job_directory Existing job directory.
# @return Lock descriptor, or None when another worker owns the job.
##
def acquire_job_lock(job_directory):
  try:
    descriptor = os.open(
      job_directory,
      os.O_RDONLY | os.O_DIRECTORY,
    )
    try:
      fcntl.flock(descriptor, fcntl.LOCK_EX | fcntl.LOCK_NB)
    except BlockingIOError:
      os.close(descriptor)
      return None
    return descriptor
  except OSError as error:
    raise AsyncJobError(f"cannot lock job directory: {error}") from error


##
# @brief Read status and reconcile an orphaned running job under its lock.
#
# @param[in] job_directory Existing job directory.
# @return Current, possibly reconciled status object.
##
def reconciled_status(job_directory):
  status_path = job_directory / "status.json"
  status = read_json(status_path)
  if status.get("state") != "running":
    return status

  lock_descriptor = acquire_job_lock(job_directory)
  if lock_descriptor is None:
    return status

  try:
    status = read_json(status_path)
    if status.get("state") == "running":
      status.update(
        {
          "error": "worker exited before publishing terminal status",
          "finished_at": utc_timestamp(),
          "state": "interrupted",
        }
      )
      write_json_atomic(status_path, status)
    return status
  finally:
    os.close(lock_descriptor)


##
# @brief Execute one persisted request and publish lifecycle state.
#
# @param[in] job_id Identifier of the job to execute.
# @return Zero after terminal state is persisted.
##
def run_worker(job_id):
  job_directory = existing_job_directory(job_id)
  lock_descriptor = acquire_job_lock(job_directory)
  if lock_descriptor is None:
    return 0

  try:
    request = read_json(job_directory / "request.json")
    status_path = job_directory / "status.json"
    initial_status = read_json(status_path)
    log_path = job_directory / "job.log"

    if initial_status.get("state") != "queued":
      return 0

    cancellation_signal_received = False

    ##
    # @brief Record a cancellation signal while allowing the worker to reap.
    #
    # @param[in] _signal_number Received signal number.
    # @param[in] _frame Interrupted Python stack frame.
    ##
    def receive_cancellation_signal(_signal_number, _frame):
      nonlocal cancellation_signal_received

      cancellation_signal_received = True

    previous_signal_handler = signal.signal(
      signal.SIGTERM, receive_cancellation_signal
    )

    try:
      initial_status["worker_pid"] = os.getpid()
      write_json_atomic(status_path, initial_status)
      if read_json(status_path).get("state") == "cancelled":
        return 0

      command = request["command"]
      command_cwd = Path(request["cwd"])
      if (
        not isinstance(command, list)
        or not command
        or not all(isinstance(item, str) for item in command)
      ):
        raise AsyncJobError("persisted command is invalid")
      resolved_cwd = resolve_command_cwd(command_cwd)

      with log_path.open("ab", buffering=0) as log_handle:
        process = subprocess.Popen(
          command,
          cwd=resolved_cwd,
          stdin=subprocess.DEVNULL,
          stdout=log_handle,
          stderr=log_handle,
          shell=False,
          close_fds=True,
        )
        running_status = dict(initial_status)
        running_status.update(
          {
            "pid": process.pid,
            "started_at": utc_timestamp(),
            "state": "running",
            "worker_pid": os.getpid(),
          }
        )
        write_json_atomic(status_path, running_status)
        exit_code = process.wait()

      current_status = read_json(status_path)
      if (
        cancellation_signal_received
        or current_status.get("state") == "cancelled"
      ):
        final_status = cancelled_status(current_status)
        final_status["exit_code"] = exit_code
      else:
        final_status = dict(running_status)
        final_status.update(
          {
            "exit_code": exit_code,
            "finished_at": utc_timestamp(),
            "state": "succeeded" if exit_code == 0 else "failed",
          }
        )
      write_json_atomic(status_path, final_status)
    except (AsyncJobError, KeyError, OSError) as error:
      current_status = read_json(status_path)
      if current_status.get("state") != "cancelled":
        failed_status = dict(initial_status)
        failed_status.update(
          {
            "error": str(error),
            "finished_at": utc_timestamp(),
            "state": "failed",
            "worker_pid": os.getpid(),
          }
        )
        write_json_atomic(status_path, failed_status)
    finally:
      signal.signal(signal.SIGTERM, previous_signal_handler)
  finally:
    os.close(lock_descriptor)

  return 0


##
# @brief Print the current status object without waiting for state changes.
#
# @param[in] job_id Identifier of the job to inspect.
##
def print_status(job_id):
  job_directory = existing_job_directory(job_id)
  status = reconciled_status(job_directory)
  json.dump(status, sys.stdout, indent=2, sort_keys=True)
  sys.stdout.write("\n")


##
# @brief Print active job identifiers and current states in stable order.
##
def print_jobs():
  build_directory = tool_build_directory()
  jobs = []
  if build_directory.is_dir() and not build_directory.is_symlink():
    for job_directory in sorted(build_directory.iterdir()):
      if (
        JOB_ID_PATTERN.fullmatch(job_directory.name) is None
        or job_directory.is_symlink()
        or not job_directory.is_dir()
      ):
        continue
      status = reconciled_status(job_directory)
      if status.get("state") in ACTIVE_STATES:
        jobs.append(
          {
            "job_id": job_directory.name,
            "state": status.get("state"),
          }
        )

  json.dump({"jobs": jobs}, sys.stdout, indent=2, sort_keys=True)
  sys.stdout.write("\n")


##
# @brief Check whether a process group still contains processes.
#
# @param[in] process_group_id Process group identifier.
# @return True while the process group exists.
##
def process_group_exists(process_group_id):
  try:
    os.killpg(process_group_id, 0)
  except ProcessLookupError:
    return False
  except PermissionError as error:
    raise AsyncJobError(
      f"cannot inspect process group {process_group_id}: {error}"
    ) from error
  return True


##
# @brief Wait for a process group to disappear within a fixed timeout.
#
# @param[in] process_group_id Process group identifier.
# @param[in] timeout_seconds Maximum wait duration.
# @return True when the process group disappeared.
##
def wait_for_process_group_exit(process_group_id, timeout_seconds):
  deadline = time.monotonic() + timeout_seconds
  while process_group_exists(process_group_id):
    if time.monotonic() >= deadline:
      return False
    time.sleep(CANCEL_CHECK_INTERVAL_SECONDS)
  return True


##
# @brief Terminate one worker-owned process group with bounded escalation.
#
# @param[in] worker_pid Worker PID and process group identifier.
##
def terminate_process_group(worker_pid):
  try:
    if os.getpgid(worker_pid) != worker_pid:
      raise AsyncJobError(
        f"worker {worker_pid} is not the expected process-group leader"
      )
    os.killpg(worker_pid, signal.SIGTERM)
  except ProcessLookupError:
    return
  except PermissionError as error:
    raise AsyncJobError(
      f"cannot signal process group {worker_pid}: {error}"
    ) from error

  if wait_for_process_group_exit(worker_pid, CANCEL_TERMINATE_TIMEOUT_SECONDS):
    return

  try:
    os.killpg(worker_pid, signal.SIGKILL)
  except ProcessLookupError:
    return
  except PermissionError as error:
    raise AsyncJobError(
      f"cannot kill process group {worker_pid}: {error}"
    ) from error

  if not wait_for_process_group_exit(worker_pid, CANCEL_KILL_TIMEOUT_SECONDS):
    raise AsyncJobError(f"process group {worker_pid} did not terminate")


##
# @brief Acquire a job lock within a fixed timeout.
#
# @param[in] job_directory Existing job directory.
# @param[in] timeout_seconds Maximum wait duration.
# @return Acquired lock descriptor, or None on timeout.
##
def acquire_job_lock_with_timeout(job_directory, timeout_seconds):
  deadline = time.monotonic() + timeout_seconds
  while True:
    lock_descriptor = acquire_job_lock(job_directory)
    if lock_descriptor is not None:
      return lock_descriptor
    if time.monotonic() >= deadline:
      return None
    time.sleep(CANCEL_CHECK_INTERVAL_SECONDS)


##
# @brief Cancel one active job and publish its terminal status.
#
# @param[in] job_id Identifier of the job to cancel.
# @return Published cancelled status object.
##
def cancel_job(job_id):
  job_directory = existing_job_directory(job_id)
  status_path = job_directory / "status.json"
  status = reconciled_status(job_directory)
  state = status.get("state")
  if state in TERMINAL_STATES:
    raise AsyncJobError(f"job {job_id} is already terminal ({state})")
  if state not in ACTIVE_STATES:
    raise AsyncJobError(f"job {job_id} has invalid state: {state}")

  lock_descriptor = acquire_job_lock(job_directory)
  if lock_descriptor is not None:
    try:
      status = read_json(status_path)
      state = status.get("state")
      if state in TERMINAL_STATES:
        raise AsyncJobError(f"job {job_id} is already terminal ({state})")
      if state not in ACTIVE_STATES:
        raise AsyncJobError(f"job {job_id} has invalid state: {state}")
      status = cancelled_status(status)
      write_json_atomic(status_path, status)
      return status
    finally:
      os.close(lock_descriptor)

  deadline = time.monotonic() + CANCEL_TERMINATE_TIMEOUT_SECONDS
  worker_pid = status.get("worker_pid")
  while not isinstance(worker_pid, int):
    if time.monotonic() >= deadline:
      break
    time.sleep(CANCEL_CHECK_INTERVAL_SECONDS)
    worker_pid = read_json(status_path).get("worker_pid")

  verification_lock = acquire_job_lock(job_directory)
  if verification_lock is not None:
    try:
      status = read_json(status_path)
      state = status.get("state")
      if state in TERMINAL_STATES:
        raise AsyncJobError(f"job {job_id} is already terminal ({state})")
      if state not in ACTIVE_STATES:
        raise AsyncJobError(f"job {job_id} has invalid state: {state}")
      status = cancelled_status(status)
      write_json_atomic(status_path, status)
      return status
    finally:
      os.close(verification_lock)

  if isinstance(worker_pid, int):
    terminate_process_group(worker_pid)

  lock_descriptor = acquire_job_lock_with_timeout(
    job_directory,
    CANCEL_TERMINATE_TIMEOUT_SECONDS + CANCEL_KILL_TIMEOUT_SECONDS,
  )
  if lock_descriptor is None:
    raise AsyncJobError(f"worker for job {job_id} did not terminate")

  try:
    status = read_json(status_path)
    state = status.get("state")
    if state in TERMINAL_STATES and state != "cancelled":
      raise AsyncJobError(f"job {job_id} is already terminal ({state})")
    if state not in ACTIVE_STATES and state != "cancelled":
      raise AsyncJobError(f"job {job_id} has invalid state: {state}")
    status = cancelled_status(status)
    write_json_atomic(status_path, status)
    return status
  finally:
    os.close(lock_descriptor)


##
# @brief Print the result of cancelling one active job as JSON.
#
# @param[in] job_id Identifier of the job to cancel.
##
def print_cancelled_status(job_id):
  status = cancel_job(job_id)
  json.dump(status, sys.stdout, indent=2, sort_keys=True)
  sys.stdout.write("\n")


##
# @brief Remove one terminal job's persisted artifacts.
#
# @param[in] job_id Identifier of the job to clean.
##
def clean_job(job_id):
  job_directory = existing_job_directory(job_id)
  status = reconciled_status(job_directory)
  state = status.get("state")
  if state in ACTIVE_STATES:
    raise AsyncJobError(f"job {job_id} is still active ({state})")
  if state not in TERMINAL_STATES:
    raise AsyncJobError(f"job {job_id} has invalid state: {state}")

  lock_descriptor = acquire_job_lock(job_directory)
  if lock_descriptor is None:
    raise AsyncJobError(f"job {job_id} is still active")
  try:
    status = read_json(job_directory / "status.json")
    state = status.get("state")
    if state in ACTIVE_STATES:
      raise AsyncJobError(f"job {job_id} is still active ({state})")
    if state not in TERMINAL_STATES:
      raise AsyncJobError(f"job {job_id} has invalid state: {state}")
    shutil.rmtree(job_directory)
  except OSError as error:
    raise AsyncJobError(
      f"cannot remove job artifacts {job_directory}: {error}"
    ) from error
  finally:
    os.close(lock_descriptor)


##
# @brief Stream a current log snapshot to standard output.
#
# @param[in] job_id Identifier of the job to inspect.
# @param[in] byte_count Maximum trailing byte count, or None for the full log.
##
def print_log(job_id, byte_count):
  job_directory = existing_job_directory(job_id)
  log_path = job_directory / "job.log"
  if log_path.is_symlink() or not log_path.is_file():
    raise AsyncJobError(f"missing or unsafe file: {log_path}")

  try:
    with log_path.open("rb") as log_handle:
      if byte_count is not None:
        log_handle.seek(0, os.SEEK_END)
        log_handle.seek(max(0, log_handle.tell() - byte_count))
      while True:
        chunk = log_handle.read(65536)
        if not chunk:
          break
        sys.stdout.buffer.write(chunk)
    sys.stdout.buffer.flush()
  except OSError as error:
    raise AsyncJobError(f"cannot read {log_path}: {error}") from error


##
# @brief Execute the selected command-line action.
#
# @param[in] arguments Optional argument list excluding the executable name.
# @return Process exit code.
##
def main(arguments=None):
  parsed_args = parse_args(arguments)

  try:
    if parsed_args.action == "start":
      job_id = start_job(
        parsed_args.executable,
        parsed_args.arguments,
        parsed_args.cwd,
      )
      print(job_id, flush=True)
    elif parsed_args.action == "status":
      print_status(parsed_args.job_id)
    elif parsed_args.action == "list":
      print_jobs()
    elif parsed_args.action == "cancel":
      print_cancelled_status(parsed_args.job_id)
    elif parsed_args.action == "clean":
      clean_job(parsed_args.job_id)
    elif parsed_args.action == "log":
      byte_count = None if parsed_args.all else parsed_args.bytes
      print_log(parsed_args.job_id, byte_count)
    else:
      return run_worker(parsed_args.job_id)
  except AsyncJobError as error:
    print(f"Error: {error}", file=sys.stderr, flush=True)
    return 2

  return 0


# ==============================================================================
# SCRIPT ENTRY POINT
# ==============================================================================

if __name__ == "__main__":
  sys.exit(main())
