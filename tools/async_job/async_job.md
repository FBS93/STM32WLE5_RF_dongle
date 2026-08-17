# async_job overview

`async_job.py` launches a command in a detached Linux session and returns a job ID immediately. It runs the command without a shell and persists its request, current state and combined standard-output/standard-error log under `build/async_job/<job-id>/`.

The tool survives the end or timeout of the Bash call that invoked `start`. Status, list and log commands return current snapshots and never wait for job completion.

# Glossary

| Term | Definition |
|---|---|
| Job ID | Random 24-character hexadecimal identifier allocated to one request. |
| Worker | Detached process that launches the requested command and records its lifecycle. |
| Active job | Job whose current state is `queued` or `running`. |

# Usage example

Start a command from the workspace root. Everything after the executable is passed as a literal argument list with `shell=False`:

```bash
job_id=$(python tools/async_job/async_job.py start -- python -c \
  'import time; time.sleep(30); print("done")')
```

Use `--cwd <path>` before `--` to select an existing working directory inside the workspace:

```bash
python tools/async_job/async_job.py start --cwd sw -- make test
```

Query the current JSON state (`queued`, `running`, `succeeded`, `failed`, `interrupted` or `cancelled`), list active jobs, cancel an active job, clean terminal artifacts, and read a non-blocking log snapshot:

```bash
python tools/async_job/async_job.py status "$job_id"
python tools/async_job/async_job.py list
python tools/async_job/async_job.py cancel "$job_id"
python tools/async_job/async_job.py clean "$job_id"
python tools/async_job/async_job.py log "$job_id"
python tools/async_job/async_job.py log "$job_id" --all
```

`log` returns at most the trailing 65536 bytes by default; use `--bytes N` to change the bound or `--all` for the complete log. Standard output and standard error are combined in production order without tool-added text.

`list` takes no options and returns only `queued` or `running` jobs as `{"jobs": [{"job_id": "...", "state": "..."}]}`, sorted by job ID. Each query reconciles persisted jobs before filtering them. A `running` job whose job-directory lock is free is atomically changed to `interrupted` and omitted. Terminal jobs (`succeeded`, `failed`, `interrupted` and `cancelled`) never appear. A free lock does not reconcile `queued`, because `start` has a narrow window before its worker acquires the lock.

`cancel JOB_ID` accepts only an active job. It atomically publishes `cancelled`, sends `SIGTERM` to the worker process group, waits for a bounded interval and uses `SIGKILL` only if the group does not exit. The worker and command share that process group, so ordinary child processes are terminated with them. The job-directory lock and persisted worker PID are checked before signalling. Cancelling a terminal job returns a clear error and leaves its artifacts unchanged.

`clean JOB_ID` accepts only a terminal job and removes only `build/async_job/<job-id>/`. Active jobs, unknown jobs and invalid states are rejected; terminated jobs are not cleaned automatically.

Each job directory contains exactly:

```text
build/async_job/<job-id>/
├── job.log
├── request.json
└── status.json
```

`request.json` stores the argument list and resolved working directory, but not the environment. Do not place secrets in command-line arguments or emit them in command output. Files are private to the creating user where filesystem permissions are enforced.

Successful `start`, `status`, `list`, `cancel` and `log` commands return exit code 0. Invalid input, an unknown job ID, a terminal cancellation request or an inaccessible artifact returns exit code 2. Terminal results remain available to `status` and `log`; jobs are not removed automatically.
