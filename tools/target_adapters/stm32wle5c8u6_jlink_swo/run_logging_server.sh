#!/usr/bin/env bash
set -euo pipefail

echo "📃 Enable STM32WLE5C8U6 J-Link SWO logging"

# Validate required environment variables.
: "${RPI_USER:?Missing RPI_USER}"
: "${RPI_HOST:?Missing RPI_HOST}"
: "${LOG_PORT:?Missing LOG_PORT}"
: "${WORKSPACE_FOLDER:?Missing WORKSPACE_FOLDER}"

# Validate required commands.
for required_command in ssh scp; do
    command -v "${required_command}" >/dev/null 2>&1 || { echo "❌ Error: ${required_command} not found."; exit 1; }
done

local_server="${WORKSPACE_FOLDER}/tools/target_adapters/stm32wle5c8u6_jlink_swo/swo_tcp_server.py"
remote_server="/tmp/stm32wle5c8u6_jlink_swo_tcp_server.py"
gdb_server_runner="${WORKSPACE_FOLDER}/.vscode/tasks/run_target_gdb_server.sh"

# Validate required files.
[ -f "${local_server}" ] || { echo "❌ Error: local server not found: ${local_server}"; exit 1; }
[ -x "${gdb_server_runner}" ] || { echo "❌ Error: GDB server runner not executable: ${gdb_server_runner}"; exit 1; }

# Let the generic GDB server runner acquire the hardware probe.
"${gdb_server_runner}"

# Deploy or reuse the project SWO TCP server.
scp -o StrictHostKeyChecking=accept-new "${local_server}" \
    "${RPI_USER}@${RPI_HOST}:${remote_server}" >/dev/null

ssh -o StrictHostKeyChecking=accept-new "${RPI_USER}@${RPI_HOST}" \
    bash -s -- "${LOG_PORT}" "${remote_server}" <<'EOF'
set -euo pipefail

log_port="$1"
remote_server="$2"
server_tag="stm32wle5c8u6-jlink_swo-v1"
pid_file="/tmp/stm32wle5c8u6_jlink_swo_tcp_server_${log_port}.pid"
log_file="/tmp/stm32wle5c8u6_jlink_swo_tcp_server_${log_port}.log"

for required_command in python3 JLinkSWOViewerCLExe; do
    command -v "${required_command}" >/dev/null 2>&1 || { echo "❌ Error: ${required_command} not found on Raspberry Pi."; exit 1; }
done

python3_path="$(command -v python3)"
listener_pids="$(/usr/bin/fuser -n tcp "${log_port}" 2>/dev/null | /usr/bin/xargs || true)"
owned_pid=""
cmdline=""
if [ -f "${pid_file}" ]; then
    owned_pid="$(<"${pid_file}")"
    [[ "${owned_pid}" =~ ^[0-9]+$ ]] && \
        cmdline="$( { /usr/bin/tr '\0' ' ' < "/proc/${owned_pid}/cmdline"; } 2>/dev/null || true)"
fi

# Reuse only the PID-file server with the exact current command.
if [ "${listener_pids}" = "${owned_pid}" ] && \
   [ "${cmdline}" = "${python3_path} ${remote_server} ${log_port} ${server_tag} " ]; then
    echo "✅ STM32WLE5C8U6 J-Link SWO TCP server already listening on port ${log_port}."
    exit 0
fi

cleanup_pids="${listener_pids}"
if [ -n "${owned_pid}" ] && [[ " ${listener_pids} " != *" ${owned_pid} "* ]] && \
   [[ "${cmdline}" == *" ${remote_server} ${log_port} "* ]]; then
    cleanup_pids="${cleanup_pids:+${cleanup_pids} }${owned_pid}"
fi

# Stop only LOG_PORT listeners and an identifiable stale owned server.
if [ -n "${cleanup_pids// /}" ]; then
    echo "Stopping stale logging PID(s): ${cleanup_pids}."
    for pid in ${cleanup_pids}; do
        /bin/kill -TERM "${pid}" 2>/dev/null || true
        for _ in {1..20}; do
            ! /bin/kill -0 "${pid}" 2>/dev/null && break
            /usr/bin/sleep 0.1
        done
        if /bin/kill -0 "${pid}" 2>/dev/null; then
            echo "Force stopping logging PID ${pid}."
            /bin/kill -KILL "${pid}" 2>/dev/null || true
        fi
    done
fi

remaining_listeners="$(/usr/bin/fuser -n tcp "${log_port}" 2>/dev/null | /usr/bin/xargs || true)"
if [ -n "${remaining_listeners}" ]; then
    echo "❌ Error: logging port ${log_port} is still occupied by PID(s): ${remaining_listeners}."
    exit 1
fi

rm -f -- "${pid_file}"
nohup "${python3_path}" "${remote_server}" "${log_port}" "${server_tag}" > "${log_file}" 2>&1 &
printf '%s\n' "$!" > "${pid_file}"
EOF
