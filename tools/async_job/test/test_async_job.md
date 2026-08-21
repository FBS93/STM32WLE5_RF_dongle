# async_job command-line interface test overview

The tests exercise the production asynchronous-job script as a black box in isolated temporary workspaces.

# Use case

Jobs are launched through `start`, inspected through non-blocking `status`, `list` and `log` calls, and stopped through `cancel`. Controlled Python commands provide successful, failed, delayed, cancellable and shell-sensitive argument cases without external dependencies.

# Verification scope

The tests verify lifecycle state and exit codes, combined logs, literal argument preservation with no shell interpretation, duplicate-worker exclusion, bounded log snapshots, unknown-job diagnostics, exact artifact layout and prompt return of `start`. They also verify that `list` contains only active jobs, terminal jobs disappear after success, failure, interruption or cancellation, orphaned `running` jobs reconcile to `interrupted`, and the free-lock `queued` startup window remains active. Cancellation tests cover queued and running jobs, concurrent completion, repeated terminal diagnostics, and termination of both a long-running command and its child process.
