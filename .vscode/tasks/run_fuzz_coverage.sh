#!/usr/bin/env bash
set -euo pipefail

echo "📊 Run fuzzer coverage"

# Validate required environment variables.
: "${FUZZ_TIMEOUT_MS:?Missing FUZZ_TIMEOUT_MS}"

# Parse command-line arguments.
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 [TARGET] [SOURCE_DIR]"
    exit 1
fi

target="$(basename "$1")"
source_dir="$2"
coverage_preset="host_fuzz_coverage"
fuzz_preset="host_fuzz"

# Validate required inputs.
if [ -z "${target}" ]; then
    echo "❌ Error: no CMake target selected."
    exit 1
fi

if [ ! -d "${source_dir}" ]; then
    echo "❌ Error: source directory not found: ${source_dir}"
    exit 1
fi

# Validate required environment variables and commands.
if ! command -v cmake >/dev/null 2>&1; then
    echo "❌ Error: cmake not found."
    exit 1
fi

if ! command -v cov-analysis >/dev/null 2>&1; then
    echo "❌ Error: cov-analysis not found. Install AFLplusplus/cov-analysis in the devcontainer."
    exit 1
fi

cd "${source_dir}"

corpus_root="build/${fuzz_preset}/afl/${target}/output/default"
queue_dir="${corpus_root}/queue"
crashes_dir="${corpus_root}/crashes"
hangs_dir="${corpus_root}/hangs"

if [ ! -d "${queue_dir}" ]; then
    echo "❌ Error: AFL corpus queue not found: ${queue_dir}"
    echo "   Run the fuzzing task first so AFL++ creates its output/default/queue directory."
    exit 1
fi

timeout_s=$(( (FUZZ_TIMEOUT_MS + 999) / 1000 ))
if [ "${timeout_s}" -lt 1 ]; then
    timeout_s=1
fi

echo "Coverage preset: ${coverage_preset}"
echo "Fuzz preset: ${fuzz_preset}"
echo "Target: ${target}"
echo "Source directory: ${source_dir}"

echo "Executing CMake ..."
# Use --no-warn-unused-cli because of this issue.
if cmake --preset "${coverage_preset}" --no-warn-unused-cli; then
    echo "✅ CMake configuration successful."
else
    echo "❌ CMake configuration failed."
    exit 1
fi

echo "Building '${target}' with LLVM source coverage instrumentation ..."
if cmake --build "build/${coverage_preset}" --target "${target}"; then
    echo "✅ Build successful for target '${target}'."
else
    echo "❌ Build failed for target '${target}'."
    exit 1
fi

target_path="$(find "build/${coverage_preset}" -type f -perm -111 -name "${target}" -print -quit)"
if [ -z "${target_path}" ]; then
    echo "❌ Error: built executable not found for target '${target}'."
    exit 1
fi

coverage_dir="build/${coverage_preset}/coverage/${target}/cov-analysis"
rm -rf "${coverage_dir}"
mkdir -p "${coverage_dir}"

analysis_input_dir="$(mktemp -d)"
trap 'rm -rf "${analysis_input_dir}"' EXIT INT TERM

queue_compat_dir="${analysis_input_dir}/queue"
mkdir -p "${queue_compat_dir}"
while IFS= read -r -d '' input_file; do
    ln -s "${PWD}/${input_file}" "${queue_compat_dir}/$(basename "${input_file}")"
done < <(find "${queue_dir}" -maxdepth 1 -type f -name 'id:*' -print0)

if [ -d "${crashes_dir}" ]; then
    crashes_compat_dir="${analysis_input_dir}/crashes"
    mkdir -p "${crashes_compat_dir}"
    while IFS= read -r -d '' input_file; do
        ln -s "${PWD}/${input_file}" "${crashes_compat_dir}/$(basename "${input_file}")"
    done < <(find "${crashes_dir}" -maxdepth 1 -type f -name 'id:*' -print0)
fi

if [ -d "${hangs_dir}" ]; then
    # cov-analysis upstream consumes AFL-style timeout artifacts from a
    # timeouts/ directory, while AFL++ writes hangs/. Keep the AFL output
    # untouched and expose hangs as timeouts only in this temporary view.
    timeouts_compat_dir="${analysis_input_dir}/timeouts"
    mkdir -p "${timeouts_compat_dir}"
    while IFS= read -r -d '' input_file; do
        ln -s "${PWD}/${input_file}" "${timeouts_compat_dir}/$(basename "${input_file}")"
    done < <(find "${hangs_dir}" -maxdepth 1 -type f -name 'id:*' -print0)
fi

echo "Executable: ${target_path}"
echo "AFL queue: ${queue_dir}"
echo "AFL crashes: ${crashes_dir}"
echo "AFL hangs: ${hangs_dir}"
echo "cov-analysis input view: ${analysis_input_dir}"
echo "Coverage directory: ${coverage_dir}"
echo "Replay timeout (s): ${timeout_s}"
echo "⚠️  Source coverage includes queue entries and crash artifacts when present."
echo "⚠️  AFL++ hangs are exposed to cov-analysis as temporary timeouts/."
echo "⚠️  This target reads stdin; cov-analysis is invoked with -e '${target_path}'."
echo "⚠️  The fuzz host port attempts a best-effort LLVM profile flush on fatal crash/timeout signals during replay."
echo "⚠️  If the binary was built without LLVM source coverage runtime support, that port hook degrades to a no-op."

cov-analysis \
    -d "${analysis_input_dir}" \
    -e "${target_path}" \
    -o "${coverage_dir}" \
    -T "${timeout_s}" \
    --layout afl
