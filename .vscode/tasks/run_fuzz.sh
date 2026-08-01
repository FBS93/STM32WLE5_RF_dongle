#!/usr/bin/env bash
set -euo pipefail

echo "🧪 Run fuzzer"

# Validate required environment variables.
: "${FUZZ_TIMEOUT_MS:?Missing FUZZ_TIMEOUT_MS}"

# Parse command-line arguments.
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 [TARGET] [SOURCE_DIR]"
    exit 1
fi

target="$(basename "$1")"
source_dir="$2"
preset="host_fuzz"

# Validate required inputs.
if [ -z "${target}" ]; then
    echo "❌ Error: no CMake target selected."
    exit 1
fi

if [ ! -d "${source_dir}" ]; then
    echo "❌ Error: source directory not found: ${source_dir}"
    exit 1
fi

# Validate required commands.
if ! command -v cmake >/dev/null 2>&1; then
    echo "❌ Error: cmake not found."
    exit 1
fi

if ! command -v afl-fuzz >/dev/null 2>&1; then
    echo "❌ Error: afl-fuzz not found."
    exit 1
fi

cd "${source_dir}"

echo "CMake preset: ${preset}"
echo "Target: ${target}"
echo "Source directory: ${source_dir}"

echo "Executing CMake ..."
# Using --no-warn-unused-cli flag because this: https://gitlab.kitware.com/cmake/cmake/-/issues/17261
if cmake --preset "${preset}" --no-warn-unused-cli; then
    echo "✅ CMake configuration successful."
else
    echo "❌ CMake configuration failed."
    exit 1
fi

echo "Building '${target}' ..."
if cmake --build "build/${preset}" --target "${target}"; then
    echo "✅ Build successful for target '${target}'."
else
    echo "❌ Build failed for target '${target}'."
    exit 1
fi

target_path="$(find "build/${preset}" -type f -perm -111 -name "${target}" -print -quit)"
if [ -z "${target_path}" ]; then
    echo "❌ Error: built executable not found for target '${target}'."
    exit 1
fi

corpus_dir="sw/fuzz/${target}/corpus"
output_dir="build/${preset}/afl/${target}/output"
mkdir -p "${corpus_dir}" "${output_dir}"

if [ -z "$(ls -A "${corpus_dir}")" ]; then
    printf '\x04\x00\x00\x00' > "${corpus_dir}/seed_min_user_signal"
fi

if [ -d "${output_dir}/default" ]; then
    afl_input_arg="-"
else
    afl_input_arg="${corpus_dir}"
fi

echo "Executable: ${target_path}"
echo "Input corpus: ${afl_input_arg}"
echo "Output directory: ${output_dir}"
echo "Hangtimeout (ms) : ${FUZZ_TIMEOUT_MS}"
echo "Starting afl-fuzz ..."
afl-fuzz -i "${afl_input_arg}" -o "${output_dir}" -t "${FUZZ_TIMEOUT_MS}" -m none -- "${target_path}"
