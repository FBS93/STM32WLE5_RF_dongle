#!/usr/bin/env bash
set -euo pipefail

echo "🧼 Format all source files"

# Parse command-line arguments.
if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
    echo "Usage: $0 [SOURCE_DIR] [WORKSPACE_ROOT] [--check]"
    exit 1
fi

source_dir="$1"
workspace_root="$2"
mode="${3-}"
clang_format_bin=""
clang_format_config="${workspace_root}/.clang-format"
cmake_format_bin=""
cmake_format_config="${workspace_root}/.cmake-format.yaml"
asm_formatter="${workspace_root}/tools/asm_format/asm_format.py"
ruff_bin=""
ruff_config="${workspace_root}/pyproject.toml"
reports_dir="${workspace_root}/build/format"
clang_report="${reports_dir}/clang_format_report.txt"
asm_report="${reports_dir}/asm_format_report.txt"
cmake_report="${reports_dir}/cmake_format_report.txt"
python_report="${reports_dir}/python_format_report.txt"
format_status=0

# Validate required inputs.
if [ -n "${mode}" ] && [ "${mode}" != "--check" ]; then
    echo "❌ Error: unsupported mode: ${mode}"
    exit 1
fi

if [ ! -d "${source_dir}" ]; then
    echo "❌ Error: source directory not found: ${source_dir}"
    exit 1
fi

if [ ! -d "${workspace_root}" ]; then
    echo "❌ Error: workspace directory not found: ${workspace_root}"
    exit 1
fi

# Resolve clang-format binary.
# 1) Use clang-format from PATH.
# 2) Fallback to VS Code extension bundled binary.
if command -v clang-format >/dev/null 2>&1; then
    clang_format_bin="$(command -v clang-format)"
else
    ext_bin=""
    extension_dirs=()
    for extension_dir in "${HOME-}/.vscode-server/extensions" "${HOME-}/.vscode/extensions"; do
        if [ -d "${extension_dir}" ]; then
            extension_dirs+=("${extension_dir}")
        fi
    done
    if [ "${#extension_dirs[@]}" -gt 0 ]; then
        ext_bin="$(
            find "${extension_dirs[@]}" -type f -name 'clang-format' \
                | sort -V | tail -n 1
        )"
    fi
    clang_format_bin="${ext_bin}"
fi

if [ -z "${clang_format_bin}" ] || [ ! -x "${clang_format_bin}" ]; then
    echo "❌ Error: Clang-format not found."
    exit 1
fi

if [ ! -f "${clang_format_config}" ]; then
    echo "❌ Error: Clang-format config file not found: ${clang_format_config}"
    exit 1
fi

if [ ! -x "${asm_formatter}" ]; then
    echo "❌ Error: ASM formatter script not found or not executable: ${asm_formatter}"
    exit 1
fi

if command -v cmake-format >/dev/null 2>&1; then
    cmake_format_bin="$(command -v cmake-format)"
fi

if [ -z "${cmake_format_bin}" ] || [ ! -x "${cmake_format_bin}" ]; then
    echo "❌ Error: Cmake-format not found."
    exit 1
fi

if [ ! -f "${cmake_format_config}" ]; then
    echo "❌ Error: Cmake-format config file not found: ${cmake_format_config}"
    exit 1
fi

if command -v ruff >/dev/null 2>&1; then
    ruff_bin="$(command -v ruff)"
fi

if [ -z "${ruff_bin}" ] || [ ! -x "${ruff_bin}" ]; then
    echo "❌ Error: Ruff formatter not found."
    exit 1
fi

if [ ! -f "${ruff_config}" ]; then
    echo "❌ Error: Ruff formatter config file not found: ${ruff_config}"
    exit 1
fi

if [ "${mode}" = "--check" ]; then
    echo "🧼 Checking formatting..."
else
    echo "🧼 Formatting..."
fi
echo "Clang-format: ${clang_format_bin}"
echo "ASM formatter: ${asm_formatter}"
echo "Cmake-format: ${cmake_format_bin}"
echo "Ruff formatter: ${ruff_bin}"

mkdir -p "${reports_dir}"
: > "${clang_report}"
: > "${asm_report}"
: > "${cmake_report}"
: > "${python_report}"

cd "${source_dir}"

if [ "${mode}" = "--check" ]; then
    # Check C source formatting recursively without modifying files.
    find . -type f \( -name '*.c' -o -name '*.h' \) \
        -exec "${clang_format_bin}" --style=file --dry-run --Werror {} + >> "${clang_report}" 2>&1 || format_status=$?

    # Check assembler source formatting recursively without modifying files.
    "${asm_formatter}" --check . >> "${asm_report}" 2>&1 || format_status=$?

    # Check CMake files across the workspace without modifying files.
    find "${workspace_root}" -type f \( -name 'CMakeLists.txt' -o -name '*.cmake' \) ! -path '*/build/*' ! -path '*/third_party/*' \
        -exec "${cmake_format_bin}" --check --config-files "${cmake_format_config}" {} + >> "${cmake_report}" 2>&1 || format_status=$?

    # Check Python formatting without modifying files.
    "${ruff_bin}" format --check --config "${ruff_config}" "${workspace_root}" >> "${python_report}" 2>&1 || format_status=$?
else
    # Format C source files recursively.
    find . -type f \( -name '*.c' -o -name '*.h' \) \
        -exec "${clang_format_bin}" --style=file -i {} + >> "${clang_report}" 2>&1 || format_status=$?

    # Format assembler source files recursively.
    find . -type f -name '*.S' -exec "${asm_formatter}" {} + >> "${asm_report}" 2>&1 || format_status=$?

    # Format CMake files across the workspace.
    find "${workspace_root}" -type f \( -name 'CMakeLists.txt' -o -name '*.cmake' \) ! -path '*/build/*' ! -path '*/third_party/*' \
        -exec "${cmake_format_bin}" --in-place --config-files "${cmake_format_config}" {} + >> "${cmake_report}" 2>&1 || format_status=$?

    # Format Python files across the workspace using the repository Ruff config.
    "${ruff_bin}" format --config "${ruff_config}" "${workspace_root}" >> "${python_report}" 2>&1 || format_status=$?
fi

if [ "${mode}" = "--check" ]; then
    echo "Formatting check completed."
else
    echo "Formatting completed."
fi
echo "Reports:"
echo "  - Clang-format: ${clang_report}"
echo "  - ASM formatter: ${asm_report}"
echo "  - Cmake-format: ${cmake_report}"
echo "  - Ruff formatter: ${python_report}"

if [ "${format_status}" -eq 0 ]; then
    echo "✅ Formatting passed."
else
    echo "❌ Formatting failed."
fi

exit "${format_status}"
