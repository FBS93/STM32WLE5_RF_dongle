#!/usr/bin/env bash
set -euo pipefail

echo "run_target_gdb_server"

# Validate required environment variables.
: "${RPI_USER:?Missing RPI_USER}"
: "${RPI_HOST:?Missing RPI_HOST}"
: "${GDB_PORT:?Missing GDB_PORT}"
: "${GDB_SERVER_RUN_CMD:?Missing GDB_SERVER_RUN_CMD}"
: "${NETWORK_LATENCY_TIMEOUT_S:?Missing NETWORK_LATENCY_TIMEOUT_S}"

# Validate required commands.
if ! command -v ssh >/dev/null 2>&1; then
    echo "❌ Error: ssh not found."
    exit 1
fi

# Log file used by the GDB server on the Raspberry Pi.
REMOTE_LOG="/tmp/run_target_gdb_server.log"

ssh -o StrictHostKeyChecking=accept-new "$RPI_USER@$RPI_HOST" bash << EOF
set -euo pipefail

# Reuse the existing GDB server if already running.
if /usr/bin/ss -ltn | /usr/bin/grep -E ":${GDB_PORT}[[:space:]]" >/dev/null; then
    echo "✅ GDB server already listening on port $GDB_PORT."
    exit 0
fi

# Kill stale instances.
/usr/bin/fuser -k ${GDB_PORT}/tcp 2>/dev/null || true

# Start the GDB server.
nohup $GDB_SERVER_RUN_CMD > "$REMOTE_LOG" 2>&1 &

# Wait for the TCP port.
TIMEOUT_MS=\$(awk 'BEGIN { print int(${NETWORK_LATENCY_TIMEOUT_S} * 1000) }')
while true; do
    if /usr/bin/ss -ltn | /usr/bin/grep -E ":${GDB_PORT}[[:space:]]" >/dev/null; then
        echo "✅ GDB server ready on port $GDB_PORT."
        exit 0
    fi
    if [ "\$TIMEOUT_MS" -le 0 ]; then
        break
    fi
    /usr/bin/sleep 0.2
    TIMEOUT_MS=\$((TIMEOUT_MS - 200))
done

echo "❌ GDB server failed."
/usr/bin/tail -n 40 "$REMOTE_LOG"
exit 1
EOF
