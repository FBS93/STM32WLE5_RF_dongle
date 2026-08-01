#!/usr/bin/env bash
set -e

# Docker named volumes can appear mounted as root-owned even when the
# devcontainer runs as a non-root user, so we fix ownership here until an
# official Docker solution is available.
runtime_root="/kicad-runtime"
config_root="${KICAD_CONFIG_HOME:-${runtime_root}/config}"
documents_root="${KICAD_DOCUMENTS_HOME:-${runtime_root}/documents}"
sudo mkdir -p "${config_root}" "${documents_root}"
sudo chown -R vscode:vscode "${runtime_root}"

# Restore KiCad state into the persistent runtime volume.
python3 "${WORKSPACE_FOLDER}/tools/kicad_state/restore_kicad_state.py"
