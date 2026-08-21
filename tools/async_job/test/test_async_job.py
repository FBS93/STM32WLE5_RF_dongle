# ==============================================================================
# @brief Black-box tests for the asynchronous job command-line interface.
#
# The tests copy the production script into temporary workspaces so generated
# build artifacts remain isolated from the repository.
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
import fcntl
import json
import os
import re
import shutil
import subprocess
import sys
import threading
import time
from pathlib import Path

# ------------------------------------------------------------------------------
# External library imports
# ------------------------------------------------------------------------------
import pytest

# ------------------------------------------------------------------------------
# Project-specific imports
# ------------------------------------------------------------------------------

# ==============================================================================
# CONSTANTS
# ==============================================================================

##
# @brief Path to the production asynchronous-job script.
##
SOURCE_SCRIPT = Path(__file__).parents[1] / "async_job.py"

##
# @brief Maximum time allowed for a test job to reach terminal state.
##
JOB_TIMEOUT_SECONDS = 5.0

# ==============================================================================
# CLASSES
# ==============================================================================

# ==============================================================================
# FUNCTIONS
# ==============================================================================


##
# @brief Create an isolated workspace containing the production script.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @return Path to the copied command-line script.
##
@pytest.fixture
def isolated_script(tmp_path):
  script_path = tmp_path / "tools" / "async_job" / "async_job.py"
  script_path.parent.mkdir(parents=True)
  shutil.copy2(SOURCE_SCRIPT, script_path)
  return script_path


##
# @brief Run one asynchronous-job command in a separate process.
#
# @param[in] script_path Path to the isolated production script.
# @param[in] arguments Command-line arguments excluding the interpreter.
# @return Completed child-process result.
##
def run_cli(script_path, *arguments):
  return subprocess.run(
    [sys.executable, str(script_path), *arguments],
    check=False,
    capture_output=True,
    text=True,
  )


##
# @brief Start one job and return its identifier and command result.
#
# @param[in] script_path Path to the isolated production script.
# @param[in] command Executable and literal argument list.
# @return Tuple containing job ID and completed start result.
##
def start_job(script_path, *command):
  result = run_cli(script_path, "start", "--", *command)
  assert result.returncode == 0, result.stderr
  job_id = result.stdout.strip()
  assert re.fullmatch(r"[0-9a-f]{24}", job_id)
  return job_id, result


##
# @brief Read the current status through the public command-line interface.
#
# @param[in] script_path Path to the isolated production script.
# @param[in] job_id Identifier returned by @c start.
# @return Parsed status object.
##
def read_status(script_path, job_id):
  result = run_cli(script_path, "status", job_id)
  assert result.returncode == 0, result.stderr
  return json.loads(result.stdout)


##
# @brief Read the active-job registry through the public interface.
#
# @param[in] script_path Path to the isolated production script.
# @return Parsed active-job list.
##
def read_active_jobs(script_path):
  result = run_cli(script_path, "list")
  assert result.returncode == 0, result.stderr
  return json.loads(result.stdout)["jobs"]


##
# @brief Wait in test code until one job reaches terminal state.
#
# @param[in] script_path Path to the isolated production script.
# @param[in] job_id Identifier returned by @c start.
# @return Terminal status object.
##
def wait_for_terminal_status(script_path, job_id):
  deadline = time.monotonic() + JOB_TIMEOUT_SECONDS
  while time.monotonic() < deadline:
    status = read_status(script_path, job_id)
    if status["state"] in (
      "succeeded",
      "failed",
      "interrupted",
      "cancelled",
    ):
      return status
    time.sleep(0.02)
  pytest.fail(f"job {job_id} did not complete")


##
# @brief Create a minimal persisted job with a selected lifecycle state.
#
# @param[in] script_path Path to the isolated production script.
# @param[in] job_id Valid job identifier.
# @param[in] state Persisted lifecycle state.
# @return Created job directory.
##
def create_persisted_job(script_path, job_id, state):
  job_directory = script_path.parents[2] / "build" / "async_job" / job_id
  job_directory.mkdir(parents=True)
  request = {
    "command": [sys.executable, "-c", "pass"],
    "cwd": str(script_path.parents[2]),
    "job_id": job_id,
    "requested_at": "2026-01-01T00:00:00.000Z",
    "schema_version": 1,
  }
  status = {
    "error": None,
    "exit_code": None,
    "finished_at": None,
    "job_id": job_id,
    "pid": 123,
    "queued_at": "2026-01-01T00:00:00.000Z",
    "schema_version": 1,
    "started_at": "2026-01-01T00:00:01.000Z",
    "state": state,
    "worker_pid": 122,
  }
  (job_directory / "request.json").write_text(
    json.dumps(request), encoding="utf-8"
  )
  (job_directory / "status.json").write_text(
    json.dumps(status), encoding="utf-8"
  )
  (job_directory / "job.log").touch()
  return job_directory


##
# @brief Check whether a PID identifies a live, non-zombie process.
#
# @param[in] process_id Process identifier to inspect.
# @return True when the process is still alive.
##
def process_is_alive(process_id):
  process_stat = Path(f"/proc/{process_id}/stat")
  try:
    fields = process_stat.read_text(encoding="utf-8").split()
  except FileNotFoundError:
    return False
  return len(fields) > 2 and fields[2] != "Z"


##
# @brief Wait briefly for one process to exit.
#
# @param[in] process_id Process identifier to inspect.
##
def assert_process_exits(process_id):
  deadline = time.monotonic() + JOB_TIMEOUT_SECONDS
  while process_is_alive(process_id):
    if time.monotonic() >= deadline:
      pytest.fail(f"process {process_id} did not exit")
    time.sleep(0.02)


##
# @brief Verify successful execution, lifecycle metadata and artifact layout.
##
def test_success_status_log_and_artifact_layout(isolated_script):
  job_id, _result = start_job(
    isolated_script,
    sys.executable,
    "-c",
    "import sys; print('out'); print('err', file=sys.stderr)",
  )

  status = wait_for_terminal_status(isolated_script, job_id)
  assert status["state"] == "succeeded"
  assert status["exit_code"] == 0
  assert isinstance(status["pid"], int)
  assert isinstance(status["worker_pid"], int)
  assert status["queued_at"].endswith("Z")
  assert status["started_at"].endswith("Z")
  assert status["finished_at"].endswith("Z")
  assert {
    job["job_id"] for job in read_active_jobs(isolated_script)
  }.isdisjoint({job_id})

  log_result = run_cli(isolated_script, "log", job_id, "--all")
  assert log_result.returncode == 0
  assert sorted(log_result.stdout.splitlines()) == ["err", "out"]

  job_directory = isolated_script.parents[2] / "build" / "async_job" / job_id
  assert {path.name for path in job_directory.iterdir()} == {
    "job.log",
    "request.json",
    "status.json",
  }


##
# @brief Verify non-zero command exits are represented as failed jobs.
##
def test_failed_command_records_exit_code(isolated_script):
  job_id, _result = start_job(
    isolated_script,
    sys.executable,
    "-c",
    "raise SystemExit(7)",
  )

  status = wait_for_terminal_status(isolated_script, job_id)
  assert status["state"] == "failed"
  assert status["exit_code"] == 7
  assert status["error"] is None
  assert {
    job["job_id"] for job in read_active_jobs(isolated_script)
  }.isdisjoint({job_id})


##
# @brief Verify shell metacharacters remain unexpanded literal arguments.
##
def test_arguments_are_not_interpreted_by_a_shell(isolated_script):
  literal_arguments = ["$(printf injected)", ";", "value with spaces", "*.py"]
  job_id, _result = start_job(
    isolated_script,
    sys.executable,
    "-c",
    "import json, sys; print(json.dumps(sys.argv[1:]))",
    *literal_arguments,
  )

  status = wait_for_terminal_status(isolated_script, job_id)
  assert status["state"] == "succeeded"
  log_result = run_cli(isolated_script, "log", job_id, "--all")
  assert json.loads(log_result.stdout) == literal_arguments

  request_path = (
    isolated_script.parents[2] / "build" / "async_job" / job_id / "request.json"
  )
  request = json.loads(request_path.read_text(encoding="utf-8"))
  assert request["command"][-len(literal_arguments) :] == literal_arguments


##
# @brief Verify a second worker cannot execute the same persisted request.
##
def test_duplicate_worker_does_not_duplicate_job(isolated_script):
  command = "import time; print('once', flush=True); time.sleep(0.5)"
  job_id, _result = start_job(
    isolated_script,
    sys.executable,
    "-c",
    command,
  )

  deadline = time.monotonic() + JOB_TIMEOUT_SECONDS
  while read_status(isolated_script, job_id)["state"] == "queued":
    if time.monotonic() >= deadline:
      pytest.fail(f"job {job_id} did not start")
    time.sleep(0.02)

  duplicate_result = run_cli(isolated_script, "_worker", job_id)
  assert duplicate_result.returncode == 0
  status = wait_for_terminal_status(isolated_script, job_id)
  assert status["state"] == "succeeded"
  log_result = run_cli(isolated_script, "log", job_id, "--all")
  assert log_result.stdout.splitlines() == ["once"]


##
# @brief Verify the detached worker continues after @c start has exited.
##
def test_job_continues_after_start_process_exits(isolated_script):
  workspace = isolated_script.parents[2]
  sentinel = workspace / "completed.txt"
  command = (
    "import pathlib, time; time.sleep(1.0); "
    "pathlib.Path('completed.txt').write_text('done', encoding='utf-8')"
  )

  started_at = time.monotonic()
  job_id, _result = start_job(
    isolated_script,
    sys.executable,
    "-c",
    command,
  )
  start_duration = time.monotonic() - started_at

  assert start_duration < 0.75
  assert not sentinel.exists()
  status = wait_for_terminal_status(isolated_script, job_id)
  assert status["state"] == "succeeded"
  assert sentinel.read_text(encoding="utf-8") == "done"


##
# @brief Verify status and bounded log queries are immediate snapshots.
##
def test_queries_and_unknown_job_exit_codes(isolated_script):
  job_id, _result = start_job(
    isolated_script,
    sys.executable,
    "-c",
    "print('0123456789')",
  )
  wait_for_terminal_status(isolated_script, job_id)

  log_result = run_cli(isolated_script, "log", job_id, "--bytes", "5")
  assert log_result.returncode == 0
  assert log_result.stdout == "6789\n"

  unknown_status = run_cli(isolated_script, "status", "0" * 24)
  unknown_log = run_cli(isolated_script, "log", "not-a-job")
  assert unknown_status.returncode == 2
  assert unknown_log.returncode == 2
  assert "unknown job ID" in unknown_status.stderr
  assert "invalid job ID" in unknown_log.stderr


##
# @brief Verify clean removes only terminal artifacts and rejects active jobs.
##
def test_clean_terminal_job_and_reject_active_or_unknown(isolated_script):
  terminal_id = "3" * 24
  terminal_directory = create_persisted_job(
    isolated_script, terminal_id, "succeeded"
  )
  other_directory = (
    isolated_script.parents[2] / "build" / "async_job" / ("4" * 24)
  )
  other_directory.mkdir()
  clean_result = run_cli(isolated_script, "clean", terminal_id)
  assert clean_result.returncode == 0, clean_result.stderr
  assert not terminal_directory.exists()
  assert other_directory.exists()

  active_id = "5" * 24
  active_directory = create_persisted_job(isolated_script, active_id, "running")
  lock_descriptor = os.open(active_directory, os.O_RDONLY | os.O_DIRECTORY)
  fcntl.flock(lock_descriptor, fcntl.LOCK_EX | fcntl.LOCK_NB)
  try:
    active_result = run_cli(isolated_script, "clean", active_id)
    assert active_result.returncode == 2
    assert "still active" in active_result.stderr
    assert active_directory.exists()
  finally:
    os.close(lock_descriptor)

  unknown_result = run_cli(isolated_script, "clean", "6" * 24)
  assert unknown_result.returncode == 2
  assert "unknown job ID" in unknown_result.stderr


##
# @brief Verify list is an active registry with running reconciliation.
##
def test_list_contains_only_active_jobs_and_reconciles_running(isolated_script):
  active_id = "1" * 24
  orphaned_id = "0" * 24
  queued_id = "2" * 24
  active_directory = create_persisted_job(isolated_script, active_id, "running")
  create_persisted_job(isolated_script, orphaned_id, "running")
  create_persisted_job(isolated_script, queued_id, "queued")
  for index, state in enumerate(
    ("succeeded", "failed", "interrupted", "cancelled"), start=3
  ):
    create_persisted_job(isolated_script, str(index) * 24, state)

  lock_descriptor = os.open(
    active_directory,
    os.O_RDONLY | os.O_DIRECTORY,
  )
  fcntl.flock(lock_descriptor, fcntl.LOCK_EX | fcntl.LOCK_NB)
  try:
    assert read_active_jobs(isolated_script) == [
      {"job_id": active_id, "state": "running"},
      {"job_id": queued_id, "state": "queued"},
    ]
  finally:
    os.close(lock_descriptor)

  interrupted = read_status(isolated_script, orphaned_id)
  assert interrupted["state"] == "interrupted"
  assert interrupted["finished_at"].endswith("Z")
  assert interrupted["error"] == (
    "worker exited before publishing terminal status"
  )


##
# @brief Verify a free lock does not misclassify the queued startup window.
##
def test_list_preserves_queued_state_with_free_lock(isolated_script):
  job_id = "2" * 24
  create_persisted_job(isolated_script, job_id, "queued")

  list_result = run_cli(isolated_script, "list")
  assert list_result.returncode == 0, list_result.stderr
  assert json.loads(list_result.stdout) == {
    "jobs": [{"job_id": job_id, "state": "queued"}]
  }
  assert read_status(isolated_script, job_id)["state"] == "queued"


##
# @brief Verify queued cancellation is atomic and repeat cancellation is safe.
##
def test_cancel_queued_job_and_diagnose_terminal_repeat(isolated_script):
  job_id = "7" * 24
  create_persisted_job(isolated_script, job_id, "queued")

  cancelled = run_cli(isolated_script, "cancel", job_id)

  assert cancelled.returncode == 0, cancelled.stderr
  assert json.loads(cancelled.stdout)["state"] == "cancelled"
  assert read_active_jobs(isolated_script) == []
  assert read_status(isolated_script, job_id)["state"] == "cancelled"

  repeated = run_cli(isolated_script, "cancel", job_id)
  assert repeated.returncode == 2
  assert "already terminal (cancelled)" in repeated.stderr
  assert read_status(isolated_script, job_id)["state"] == "cancelled"


##
# @brief Verify cancellation handles a queued job whose worker owns the lock.
##
def test_cancel_queued_worker_lock_window(isolated_script):
  job_id = "8" * 24
  job_directory = create_persisted_job(isolated_script, job_id, "queued")
  ready_path = isolated_script.parents[2] / "worker-ready"
  holder_code = (
    "import fcntl, os, pathlib, time; "
    "descriptor = os.open(os.environ['JOB_DIRECTORY'], "
    "os.O_RDONLY | os.O_DIRECTORY); "
    "fcntl.flock(descriptor, fcntl.LOCK_EX); "
    "pathlib.Path(os.environ['READY_PATH']).touch(); "
    "time.sleep(30)"
  )
  environment = os.environ.copy()
  environment["JOB_DIRECTORY"] = str(job_directory)
  environment["READY_PATH"] = str(ready_path)
  holder = subprocess.Popen(
    [sys.executable, "-c", holder_code],
    env=environment,
    start_new_session=True,
  )

  try:
    deadline = time.monotonic() + JOB_TIMEOUT_SECONDS
    while not ready_path.exists():
      if time.monotonic() >= deadline:
        pytest.fail("queued worker did not acquire its lock")
      time.sleep(0.02)

    status_path = job_directory / "status.json"
    status = json.loads(status_path.read_text(encoding="utf-8"))
    status["worker_pid"] = holder.pid
    status_path.write_text(json.dumps(status), encoding="utf-8")
    reaper = threading.Thread(target=holder.wait)
    reaper.start()

    cancelled = run_cli(isolated_script, "cancel", job_id)

    assert cancelled.returncode == 0, cancelled.stderr
    assert json.loads(cancelled.stdout)["state"] == "cancelled"
    assert read_active_jobs(isolated_script) == []
    assert_process_exits(holder.pid)
    reaper.join(timeout=JOB_TIMEOUT_SECONDS)
  finally:
    if process_is_alive(holder.pid):
      os.killpg(holder.pid, 9)
    holder.wait(timeout=JOB_TIMEOUT_SECONDS)


##
# @brief Verify cancellation cannot overwrite a concurrently completed job.
##
def test_cancel_does_not_overwrite_concurrent_success(isolated_script):
  job_id = "9" * 24
  job_directory = create_persisted_job(isolated_script, job_id, "running")
  ready_path = isolated_script.parents[2] / "worker-ready"
  holder_code = (
    "import fcntl, json, os, pathlib, signal, sys, time\n"
    "job_directory = pathlib.Path(os.environ['JOB_DIRECTORY'])\n"
    "status_path = job_directory / 'status.json'\n"
    "descriptor = os.open(job_directory, os.O_RDONLY | os.O_DIRECTORY)\n"
    "fcntl.flock(descriptor, fcntl.LOCK_EX)\n"
    "status = json.loads(status_path.read_text(encoding='utf-8'))\n"
    "status['worker_pid'] = os.getpid()\n"
    "status_path.write_text(json.dumps(status), encoding='utf-8')\n"
    "pathlib.Path(os.environ['READY_PATH']).touch()\n"
    "def finish(_signal, _frame):\n"
    "  status = json.loads(status_path.read_text(encoding='utf-8'))\n"
    "  status.update({'exit_code': 0, 'finished_at': "
    "'2026-01-01T00:00:02.000Z', 'state': 'succeeded'})\n"
    "  status_path.write_text(json.dumps(status), encoding='utf-8')\n"
    "  sys.exit(0)\n"
    "signal.signal(signal.SIGTERM, finish)\n"
    "time.sleep(30)\n"
  )
  environment = os.environ.copy()
  environment["JOB_DIRECTORY"] = str(job_directory)
  environment["READY_PATH"] = str(ready_path)
  holder = subprocess.Popen(
    [sys.executable, "-c", holder_code],
    env=environment,
    start_new_session=True,
  )

  try:
    deadline = time.monotonic() + JOB_TIMEOUT_SECONDS
    while not ready_path.exists():
      if time.monotonic() >= deadline:
        pytest.fail("concurrent worker did not acquire its lock")
      time.sleep(0.02)

    reaper = threading.Thread(target=holder.wait)
    reaper.start()
    cancelled = run_cli(isolated_script, "cancel", job_id)

    assert cancelled.returncode == 2
    assert "already terminal (succeeded)" in cancelled.stderr
    assert read_status(isolated_script, job_id)["state"] == "succeeded"
    reaper.join(timeout=JOB_TIMEOUT_SECONDS)
  finally:
    if process_is_alive(holder.pid):
      os.killpg(holder.pid, 9)
    holder.wait(timeout=JOB_TIMEOUT_SECONDS)


##
# @brief Verify cancellation terminates a command and its child process.
##
def test_cancel_running_job_leaves_no_live_processes(isolated_script):
  workspace = isolated_script.parents[2]
  child_pid_path = workspace / "child.pid"
  command = (
    "import pathlib, subprocess, time; "
    "child = subprocess.Popen(['sleep', '30']); "
    "pathlib.Path('child.pid').write_text(str(child.pid), encoding='utf-8'); "
    "time.sleep(30)"
  )
  job_id, _result = start_job(
    isolated_script,
    sys.executable,
    "-c",
    command,
  )

  deadline = time.monotonic() + JOB_TIMEOUT_SECONDS
  status = read_status(isolated_script, job_id)
  while status["state"] != "running" or not child_pid_path.exists():
    if time.monotonic() >= deadline:
      pytest.fail(f"job {job_id} did not start its child")
    time.sleep(0.02)
    status = read_status(isolated_script, job_id)

  command_pid = status["pid"]
  worker_pid = status["worker_pid"]
  child_pid = int(child_pid_path.read_text(encoding="utf-8"))
  cancelled = run_cli(isolated_script, "cancel", job_id)

  assert cancelled.returncode == 0, cancelled.stderr
  assert json.loads(cancelled.stdout)["state"] == "cancelled"
  assert read_active_jobs(isolated_script) == []
  assert_process_exits(worker_pid)
  assert_process_exits(command_pid)
  assert_process_exits(child_pid)
