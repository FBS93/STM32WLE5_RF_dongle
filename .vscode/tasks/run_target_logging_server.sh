#!/usr/bin/env bash
set -euo pipefail

echo "📃 Enable target logging"

# Validate required environment variables.
: "${RPI_USER:?Missing RPI_USER}"
: "${RPI_HOST:?Missing RPI_HOST}"
: "${LOG_PORT:?Missing LOG_PORT}"
: "${LOG_BAUD_RATE:?Missing LOG_BAUD_RATE}"
: "${LOG_SERIAL_DEVICE:?Missing LOG_SERIAL_DEVICE}"
: "${NETWORK_LATENCY_TIMEOUT_S:?Missing NETWORK_LATENCY_TIMEOUT_S}"
: "${WORKSPACE_FOLDER:?Missing WORKSPACE_FOLDER}"

# Validate required commands.
if ! command -v ssh >/dev/null 2>&1; then
    echo "❌ Error: ssh not found."
    exit 1
fi

if ! command -v scp >/dev/null 2>&1; then
    echo "❌ Error: scp not found."
    exit 1
fi

# Local and remote file paths used to deploy and run the logging server.
LOCAL_SCRIPT="${WORKSPACE_FOLDER}/tools/run_target_logging_server/run_target_logging_server.py"
REMOTE_SCRIPT="/tmp/run_target_logging_server.py"
REMOTE_LOG="/tmp/run_target_logging_server.log"

if [ ! -f "${LOCAL_SCRIPT}" ]; then
    echo "❌ Error: local script not found: ${LOCAL_SCRIPT}"
    exit 1
fi

# Copy the target logging server script to the Raspberry Pi.
scp -o StrictHostKeyChecking=accept-new "${LOCAL_SCRIPT}" "${RPI_USER}@${RPI_HOST}:${REMOTE_SCRIPT}" >/dev/null

ssh -o StrictHostKeyChecking=accept-new "${RPI_USER}@${RPI_HOST}" bash << EOF
set -euo pipefail

# Use the configured serial device.
if [ ! -e "${LOG_SERIAL_DEVICE}" ]; then
    echo "❌ Configured logging serial device not found: ${LOG_SERIAL_DEVICE}"
    exit 1
fi

SERIAL_DEVICE="\$(/usr/bin/readlink -f -- "${LOG_SERIAL_DEVICE}")"
echo "USB serial device configured: ${LOG_SERIAL_DEVICE}."

# Reuse the existing logging server when healthy and matching the device and port.
if /usr/bin/ss -ltn | /usr/bin/grep ":$LOG_PORT" >/dev/null && \
   /usr/bin/pgrep -f "python3 ${REMOTE_SCRIPT} ${LOG_SERIAL_DEVICE} ${LOG_PORT} ${LOG_BAUD_RATE}" >/dev/null; then
    for pid in \$(/usr/bin/pgrep -f "python3 ${REMOTE_SCRIPT} ${LOG_SERIAL_DEVICE} ${LOG_PORT} ${LOG_BAUD_RATE}"); do
        for fd in /proc/\${pid}/fd/*; do
            if [ "\$(/usr/bin/readlink -f -- "\${fd}" 2>/dev/null)" = "\${SERIAL_DEVICE}" ]; then
                if bash -c "exec 9<>/dev/tcp/127.0.0.1/$LOG_PORT" 2>/dev/null; then
                    exec 9>&-
                    exec 9<&-
                    echo "✅ Logging server already listening on port $LOG_PORT."
                    exit 0
                fi
            fi
        done
    done
fi

# Stop stale instances before starting a fresh one.
if /usr/bin/ss -ltn | /usr/bin/grep ":$LOG_PORT" >/dev/null; then
    /usr/bin/fuser -k ${LOG_PORT}/tcp 2>/dev/null || true
fi
/usr/bin/pkill -f "python3 ${REMOTE_SCRIPT}" 2>/dev/null || true

# Start the logging server.
nohup python3 "${REMOTE_SCRIPT}" "${LOG_SERIAL_DEVICE}" "${LOG_PORT}" "${LOG_BAUD_RATE}" \
    > "${REMOTE_LOG}" 2>&1 &

# Wait for the TCP port.
TIMEOUT_MS=\$(awk 'BEGIN { print int(${NETWORK_LATENCY_TIMEOUT_S} * 1000) }')
while true; do
    if /usr/bin/ss -ltn | /usr/bin/grep ":$LOG_PORT" >/dev/null; then
        echo "✅ Logging server ready on port $LOG_PORT."
        exit 0
    fi
    if [ "\$TIMEOUT_MS" -le 0 ]; then
        break
    fi
    /usr/bin/sleep 0.2
    TIMEOUT_MS=\$((TIMEOUT_MS - 200))
done

echo "❌ Logging server failed."
/usr/bin/tail -n 40 "${REMOTE_LOG}" || true
exit 1
EOF
