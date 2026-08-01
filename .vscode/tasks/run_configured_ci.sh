#!/usr/bin/env bash
set -euo pipefail

# Parse command-line arguments.
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 [WORKSPACE_ROOT]"
    exit 1
fi

workspace_root="${1%/}"
configured_presets=()

# Validate required environment variables.
if [ -z "${CI_PRESETS-}" ]; then
    echo "❌ Error: CI_PRESETS must list at least one CMake preset."
    exit 1
fi

read -r -a configured_presets <<< "${CI_PRESETS}"

if [ "${#configured_presets[@]}" -eq 0 ]; then
    echo "❌ Error: CI_PRESETS must list at least one CMake preset."
    exit 1
fi

"${workspace_root}/.vscode/tasks/run_ci.sh" "${workspace_root}" \
    "${configured_presets[@]}"
