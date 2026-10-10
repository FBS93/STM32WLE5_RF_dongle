#!/usr/bin/env bash
set -euo pipefail

echo "📃 Enable STM32WLE5C8U6 J-Link RTT logging"

: "${RPI_USER:?Missing RPI_USER}"
: "${RPI_HOST:?Missing RPI_HOST}"
: "${GDB_PORT:?Missing GDB_PORT}"
: "${GDB_SERVER_RUN_CMD:?Missing GDB_SERVER_RUN_CMD}"
: "${LOG_PORT:?Missing LOG_PORT}"
: "${NETWORK_LATENCY_TIMEOUT_S:?Missing NETWORK_LATENCY_TIMEOUT_S}"
: "${WORKSPACE_FOLDER:?Missing WORKSPACE_FOLDER}"

if ! command -v ssh >/dev/null 2>&1; then
    echo "❌ Error: ssh not found."
    exit 1
fi

gdb_server_runner="${WORKSPACE_FOLDER}/.vscode/tasks/run_target_gdb_server.sh"
if [ ! -x "${gdb_server_runner}" ]; then
    echo "❌ Error: GDB server runner not executable: ${gdb_server_runner}"
    exit 1
fi

# The generic runner owns the JLinkGDBServer lifecycle and reuses an existing
# server. RTT is exposed directly by that same process; no data process or
# debugger connection is created here.
"${gdb_server_runner}"

ssh -o StrictHostKeyChecking=accept-new "${RPI_USER}@${RPI_HOST}" \
    bash -s -- "${GDB_PORT}" "${LOG_PORT}" <<'EOF'
set -euo pipefail

gdb_port="$1"
rtt_port="$2"
gdb_pid="$(/usr/bin/fuser -n tcp "${gdb_port}" 2>/dev/null | /usr/bin/xargs || true)"
rtt_pid="$(/usr/bin/fuser -n tcp "${rtt_port}" 2>/dev/null | /usr/bin/xargs || true)"

if [ -z "${gdb_pid}" ] || [ "${gdb_pid}" != "${rtt_pid}" ]; then
    echo "❌ Error: GDB and RTT ports are not owned by one JLinkGDBServer."
    exit 1
fi

cmdline="$(/usr/bin/tr '\0' ' ' < "/proc/${gdb_pid}/cmdline")"
if [[ "${cmdline}" != *"JLinkGDBServer"* ]]; then
    echo "❌ Error: port owner is not JLinkGDBServer: ${cmdline}"
    exit 1
fi

echo "✅ J-Link RTT logging available on port ${rtt_port} (PID ${gdb_pid})."
EOF
