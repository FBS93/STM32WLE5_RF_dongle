#!/usr/bin/env bash
set -euo pipefail

echo "🔎 Run linter"

# Parse command-line arguments.
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 [WORKSPACE_ROOT] [PRESET...]"
    exit 1
fi

workspace_root="${1%/}"
shift
presets=("$@")
ruff_bin=""
ruff_config="${workspace_root}/pyproject.toml"

# Validate required inputs.
if [ ! -d "${workspace_root}" ]; then
    echo "❌ Error: workspace directory not found: ${workspace_root}"
    exit 1
fi

for preset in "${presets[@]}"; do
    if [ -z "${preset}" ]; then
        echo "❌ Error: no CMake preset selected."
        exit 1
    fi
done

# Validate required commands.
if ! command -v cmake >/dev/null 2>&1; then
    echo "❌ Error: cmake not found."
    exit 1
fi

if command -v ruff >/dev/null 2>&1; then
    ruff_bin="$(command -v ruff)"
fi

if [ -z "${ruff_bin}" ] || [ ! -x "${ruff_bin}" ]; then
    echo "❌ Error: Ruff linter not found."
    exit 1
fi

if [ ! -f "${ruff_config}" ]; then
    echo "❌ Error: Ruff linter config file not found: ${ruff_config}"
    exit 1
fi

echo "CMake presets: ${presets[*]}"
echo "Ruff linter: ${ruff_bin}"

cd "${workspace_root}"

for preset in "${presets[@]}"; do
    build_dir="${workspace_root}/build/${preset}"

    echo "Configuring preset '${preset}' with clang-tidy enabled ..."
    # Using --no-warn-unused-cli flag because this: https://gitlab.kitware.com/cmake/cmake/-/issues/17261
    if cmake --preset "${preset}" -DEW_ENABLE_CLANG_TIDY=ON --no-warn-unused-cli; then
        echo "✅ CMake configuration successful for preset '${preset}'."
    else
        echo "❌ CMake configuration failed for preset '${preset}'."
        exit 1
    fi

    echo "Building preset '${preset}' with clang-tidy enabled ..."
    if cmake --build "${build_dir}" --clean-first; then
        echo "✅ Lint build successful for preset '${preset}'."
    else
        echo "❌ Lint build failed for preset '${preset}'."
        exit 1
    fi
done

echo "Running Python lint with Ruff ..."
if "${ruff_bin}" check --config "${ruff_config}" "${workspace_root}"; then
    echo "✅ Python lint successful."
else
    echo "❌ Python lint failed."
    exit 1
fi
