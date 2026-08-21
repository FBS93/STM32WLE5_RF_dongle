#!/usr/bin/env bash
set -euo pipefail

echo "☑️ Validate release"

# Parse command-line arguments.
if [ "$#" -ne 3 ]; then
    echo "Usage: $0 [WORKSPACE_ROOT] [PROJECT_TITLE] [VERSION]"
    exit 1
fi

workspace_root="${1%/}"
project_title="$2"
release_version="$3"
ci_log="build/release-${release_version}-ci.log"

# Validate required inputs.
if [ ! -d "${workspace_root}" ]; then
    echo "❌ Error: workspace directory not found: ${workspace_root}"
    exit 1
fi

if [[ ! "${release_version}" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
    echo "❌ Error: release version must match vX.Y.Z."
    exit 1
fi

# Validate required environment variables.
if [ -z "${DOCGEN_OUTPUT-}" ]; then
    echo "❌ Error: DOCGEN_OUTPUT must define the documentation output directory."
    exit 1
fi

# Validate required commands.
if ! command -v python3 >/dev/null 2>&1; then
    echo "❌ Error: python3 not found."
    exit 1
fi

cd "${workspace_root}"
mkdir -p "${ci_log%/*}"

# Retain CI output for generated documentation while preserving CI's status.
echo "Running CI for configured presets..."
if ./.vscode/tasks/run_configured_ci.sh "${workspace_root}" > "${ci_log}" 2>&1; then
    ci_status=0
else
    ci_status=$?
fi
cat "${ci_log}"

if [ "${ci_status}" -ne 0 ]; then
    echo "❌ CI failed with status ${ci_status}. Log: ${ci_log}" >&2
fi

echo "Generating documentation with CI results..."
if python3 "tools/docgen/docgen.py" --root . --output "${DOCGEN_OUTPUT}" \
    --title "${project_title} ${release_version}" --exclude third_party --strict \
    --test-result "CI" "${ci_log}"; then
    echo "✅ Generated: ${DOCGEN_OUTPUT}/index.html"
else
    status=$?
    echo "❌ Documentation generation failed with status ${status}." >&2
    exit "${status}"
fi

exit "${ci_status}"
