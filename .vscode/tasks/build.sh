#!/usr/bin/env bash
set -euo pipefail

echo "🔨 Build"

# Parse command-line arguments.
if [ "$#" -lt 3 ]; then
    echo "Usage: $0 [TARGET] [PRESET] [SOURCE_DIR]"
    exit 1
fi

target="$(basename "$1")"
preset="$2"
source_dir="$3"

# Validate required inputs.
if [ -z "${preset}" ]; then
    echo "❌ Error: no CMake preset selected."
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

echo "CMake preset: ${preset}"
echo "Target: ${target}"
echo "Source directory: ${source_dir}"

cd "${source_dir}"

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
