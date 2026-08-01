#!/usr/bin/env bash
set -euo pipefail

echo "📈 Run PulseView web"

# Validate required environment variables.
: "${RPI_USER:?Missing RPI_USER}"
: "${RPI_HOST:?Missing RPI_HOST}"
: "${LOGIC_ANALYZER_DEVICE:?Missing LOGIC_ANALYZER_DEVICE}"
: "${LOGIC_ANALYZER_ARTIFACTS_DIR:?Missing LOGIC_ANALYZER_ARTIFACTS_DIR}"
: "${WORKSPACE_FOLDER:?Missing WORKSPACE_FOLDER}"

artifacts_relative_path="${LOGIC_ANALYZER_ARTIFACTS_DIR#${WORKSPACE_FOLDER}/}"

if [ "${artifacts_relative_path}" = "${LOGIC_ANALYZER_ARTIFACTS_DIR}" ]; then
    echo "❌ Error: LOGIC_ANALYZER_ARTIFACTS_DIR must be located inside WORKSPACE_FOLDER."
    exit 1
fi

remote_display=":99"
remote_runtime_dir="/tmp/embedded_workbench_logic_analyzer"
remote_vnc_port="5901"
remote_web_port="6080"
remote_capture_root="/tmp/${artifacts_relative_path}"

# Validate required commands.
if ! command -v ssh >/dev/null 2>&1; then
    echo "❌ Error: ssh not found."
    exit 1
fi

ssh -o StrictHostKeyChecking=accept-new "${RPI_USER}@${RPI_HOST}" \
    REMOTE_DISPLAY="${remote_display}" \
    REMOTE_RUNTIME_DIR="${remote_runtime_dir}" \
    REMOTE_VNC_PORT="${remote_vnc_port}" \
    REMOTE_WEB_PORT="${remote_web_port}" \
    REMOTE_CAPTURE_ROOT="${remote_capture_root}" \
    LOGIC_ANALYZER_DEVICE="${LOGIC_ANALYZER_DEVICE}" \
    RPI_HOST_VALUE="${RPI_HOST}" \
    'bash -s' <<'EOF'
set -euo pipefail

for required_command in pulseview Xvfb x0tigervncserver websockify openbox wmctrl; do
    if ! command -v "${required_command}" >/dev/null 2>&1; then
        echo "❌ Error: missing remote dependency: ${required_command}"
        exit 1
    fi
done

if [ ! -d /usr/share/novnc ]; then
    echo "❌ Error: missing noVNC web root: /usr/share/novnc"
    exit 1
fi

mkdir -p "${REMOTE_RUNTIME_DIR}"

stop_pid_file() {
    pid_file="$1"

    if [ -f "${pid_file}" ]; then
        pid="$(cat "${pid_file}")"
        if kill -0 "${pid}" >/dev/null 2>&1; then
            kill "${pid}" >/dev/null 2>&1 || true
            wait "${pid}" 2>/dev/null || true
        fi
        rm -f "${pid_file}"
    fi
}

port_is_listening() {
    local port="$1"

    if (exec 3<>"/dev/tcp/127.0.0.1/${port}") >/dev/null 2>&1; then
        exec 3>&-
        exec 3<&-
        return 0
    fi

    return 1
}

wait_for_process() {
    local pid_file="$1"
    local service_name="$2"
    local log_file="$3"
    local conflict_hint="${4-}"
    local attempt=0

    while [ "${attempt}" -lt 10 ]; do
        if [ ! -f "${pid_file}" ]; then
            echo "❌ Error: ${service_name} pid file was not created: ${pid_file}"
            exit 1
        fi

        pid="$(cat "${pid_file}")"
        if kill -0 "${pid}" >/dev/null 2>&1; then
            return 0
        fi

        sleep 1
        attempt=$((attempt + 1))
    done

    echo "❌ Error: ${service_name} exited during startup."
    if [ -n "${conflict_hint}" ] && [ -e "${conflict_hint}" ]; then
        echo "Likely cause: ${conflict_hint} already exists on the Raspberry Pi."
    fi
    if [ -f "${log_file}" ]; then
        echo "Log tail (${log_file}):"
        tail -n 20 "${log_file}" || true
    fi
    exit 1
}

wait_for_port() {
    local port="$1"
    local service_name="$2"
    local log_file="$3"
    local attempt=0

    while [ "${attempt}" -lt 10 ]; do
        if port_is_listening "${port}"; then
            return 0
        fi

        sleep 1
        attempt=$((attempt + 1))
    done

    echo "❌ Error: ${service_name} is not listening on port ${port}."
    if [ -f "${log_file}" ]; then
        echo "Log tail (${log_file}):"
        tail -n 20 "${log_file}" || true
    fi
    exit 1
}

wait_for_window_by_pid() {
    local target_pid="$1"
    local service_name="$2"
    local log_file="$3"
    local attempt=0

    while [ "${attempt}" -lt 15 ]; do
        window_id="$({
            env DISPLAY="${REMOTE_DISPLAY}" wmctrl -lp 2>/dev/null || true
        } | awk -v target_pid="${target_pid}" '$3 == target_pid { print $1; exit }')"

        if [ -z "${window_id}" ]; then
            window_id="$({
                env DISPLAY="${REMOTE_DISPLAY}" wmctrl -lp 2>/dev/null || true
            } | awk 'index($0, "PulseView") { print $1; exit }')"
        fi

        if [ -n "${window_id}" ]; then
            printf '%s\n' "${window_id}"
            return 0
        fi

        sleep 1
        attempt=$((attempt + 1))
    done

    echo "❌ Error: ${service_name} window did not appear on display ${REMOTE_DISPLAY}."
    if [ -f "${log_file}" ]; then
        echo "Log tail (${log_file}):"
        tail -n 20 "${log_file}" || true
    fi
    exit 1
}

maximize_window() {
    local window_id="$1"
    local service_name="$2"
    local log_file="$3"

    if ! env DISPLAY="${REMOTE_DISPLAY}" wmctrl -i -r "${window_id}" -b add,maximized_vert,maximized_horz; then
        echo "❌ Error: failed to maximize ${service_name} window ${window_id}."
        if [ -f "${log_file}" ]; then
            echo "Log tail (${log_file}):"
            tail -n 20 "${log_file}" || true
        fi
        exit 1
    fi
}

cleanup_started_services() {
    stop_pid_file "${REMOTE_RUNTIME_DIR}/pulseview.pid"
    stop_pid_file "${REMOTE_RUNTIME_DIR}/openbox.pid"
    stop_pid_file "${REMOTE_RUNTIME_DIR}/websockify.pid"
    stop_pid_file "${REMOTE_RUNTIME_DIR}/x0tigervncserver.pid"
    stop_pid_file "${REMOTE_RUNTIME_DIR}/xvfb.pid"
}

trap 'cleanup_started_services' ERR

stop_pid_file "${REMOTE_RUNTIME_DIR}/pulseview.pid"
stop_pid_file "${REMOTE_RUNTIME_DIR}/openbox.pid"
stop_pid_file "${REMOTE_RUNTIME_DIR}/websockify.pid"
stop_pid_file "${REMOTE_RUNTIME_DIR}/x0tigervncserver.pid"
stop_pid_file "${REMOTE_RUNTIME_DIR}/xvfb.pid"

# Stop stale workflow instances even if PID files were lost.
if port_is_listening "${REMOTE_VNC_PORT}"; then
    /usr/bin/fuser -k "${REMOTE_VNC_PORT}"/tcp 2>/dev/null || true
fi

if port_is_listening "${REMOTE_WEB_PORT}"; then
    /usr/bin/fuser -k "${REMOTE_WEB_PORT}"/tcp 2>/dev/null || true
fi

/usr/bin/pkill -f "Xvfb ${REMOTE_DISPLAY} -screen 0 1920x1080x24" 2>/dev/null || true
/usr/bin/pkill -f "openbox --sm-disable" 2>/dev/null || true
/usr/bin/pkill -f "x0tigervncserver -display ${REMOTE_DISPLAY} -rfbport ${REMOTE_VNC_PORT}" 2>/dev/null || true
/usr/bin/pkill -f "websockify --web /usr/share/novnc ${REMOTE_WEB_PORT} localhost:${REMOTE_VNC_PORT}" 2>/dev/null || true
/usr/bin/pkill -f "pulseview -c -D -d ${LOGIC_ANALYZER_DEVICE}" 2>/dev/null || true

nohup Xvfb "${REMOTE_DISPLAY}" -screen 0 1920x1080x24 \
    >"${REMOTE_RUNTIME_DIR}/xvfb.log" 2>&1 </dev/null &
printf '%s\n' "$!" >"${REMOTE_RUNTIME_DIR}/xvfb.pid"

wait_for_process "${REMOTE_RUNTIME_DIR}/xvfb.pid" "Xvfb" "${REMOTE_RUNTIME_DIR}/xvfb.log" "/tmp/.X11-unix/X${REMOTE_DISPLAY#:}"

nohup env DISPLAY="${REMOTE_DISPLAY}" openbox --sm-disable \
    >"${REMOTE_RUNTIME_DIR}/openbox.log" 2>&1 </dev/null &
printf '%s\n' "$!" >"${REMOTE_RUNTIME_DIR}/openbox.pid"

wait_for_process "${REMOTE_RUNTIME_DIR}/openbox.pid" "Openbox" "${REMOTE_RUNTIME_DIR}/openbox.log"

nohup x0tigervncserver -display "${REMOTE_DISPLAY}" \
    -rfbport "${REMOTE_VNC_PORT}" \
    -localhost yes \
    -SecurityTypes None \
    >"${REMOTE_RUNTIME_DIR}/tigervnc.log" 2>&1 </dev/null &
printf '%s\n' "$!" >"${REMOTE_RUNTIME_DIR}/x0tigervncserver.pid"

wait_for_process "${REMOTE_RUNTIME_DIR}/x0tigervncserver.pid" "TigerVNC" "${REMOTE_RUNTIME_DIR}/tigervnc.log"
wait_for_port "${REMOTE_VNC_PORT}" "TigerVNC" "${REMOTE_RUNTIME_DIR}/tigervnc.log"

nohup websockify --web /usr/share/novnc "${REMOTE_WEB_PORT}" \
    "localhost:${REMOTE_VNC_PORT}" \
    >"${REMOTE_RUNTIME_DIR}/websockify.log" 2>&1 </dev/null &
printf '%s\n' "$!" >"${REMOTE_RUNTIME_DIR}/websockify.pid"

wait_for_process "${REMOTE_RUNTIME_DIR}/websockify.pid" "websockify" "${REMOTE_RUNTIME_DIR}/websockify.log"
wait_for_port "${REMOTE_WEB_PORT}" "websockify" "${REMOTE_RUNTIME_DIR}/websockify.log"

nohup env DISPLAY="${REMOTE_DISPLAY}" pulseview \
    -c \
    -D \
    -d "${LOGIC_ANALYZER_DEVICE}" \
    >"${REMOTE_RUNTIME_DIR}/pulseview.log" 2>&1 </dev/null &
printf '%s\n' "$!" >"${REMOTE_RUNTIME_DIR}/pulseview.pid"

wait_for_process "${REMOTE_RUNTIME_DIR}/pulseview.pid" "PulseView" "${REMOTE_RUNTIME_DIR}/pulseview.log"

pulseview_pid="$(cat "${REMOTE_RUNTIME_DIR}/pulseview.pid")"
pulseview_window_id="$(wait_for_window_by_pid "${pulseview_pid}" "PulseView" "${REMOTE_RUNTIME_DIR}/pulseview.log")"
maximize_window "${pulseview_window_id}" "PulseView" "${REMOTE_RUNTIME_DIR}/pulseview.log"

trap - ERR

detected_ip=""
if detected_ips="$(hostname -I 2>/dev/null)" && [ -n "${detected_ips}" ]; then
    set -- ${detected_ips}
    detected_ip="$1"
fi

echo "✅ PulseView web is ready."
echo "URL: http://${RPI_HOST_VALUE}:${REMOTE_WEB_PORT}/vnc.html?autoconnect=true&resize=scale"
if [ -n "${detected_ip}" ] && [ "${detected_ip}" != "${RPI_HOST_VALUE}" ]; then
    echo "URL (detected Raspberry Pi IP): http://${detected_ip}:${REMOTE_WEB_PORT}/vnc.html?autoconnect=true&resize=scale"
fi
echo "Remote capture root: ${REMOTE_CAPTURE_ROOT}"
echo "Logs: ${REMOTE_RUNTIME_DIR}"
EOF
