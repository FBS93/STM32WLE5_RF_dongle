#!/usr/bin/env bash
set -euo pipefail

# Parse command-line arguments.
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 [WORKSPACE_ROOT] [PRESET...]"
    exit 1
fi

workspace_root="${1%/}"
shift
presets=("$@")
line_width=80
header_footer_separator="$(printf '%*s' "${line_width}" '' | tr ' ' '=')"
section_separator="$(printf '%*s' "${line_width}" '' | tr ' ' '-')"

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

print_centered() {
    local title="$1"
    local padding=$(((line_width - ${#title}) / 2))

    printf '%*s%s%*s\n' "${padding}" '' "${title}" \
        "$((line_width - ${#title} - padding))" ''
}

print_section() {
    local title="$1"
    local padding
    local right_padding

    if [ $(( ${#title} + 2 )) -gt "${line_width}" ]; then
        printf '\n%s\n\n' "${title}"
        return
    fi

    padding=$(((line_width - ${#title} - 2) / 2))
    right_padding=$((line_width - ${#title} - 2 - padding))

    printf '\n%s %s %s\n\n' "${section_separator:0:padding}" "${title}" \
        "${section_separator:0:right_padding}"
}

print_ci_footer() {
    local result="$1"

    printf '\n%s\n' "${header_footer_separator}"
    print_centered "CI ${result}"
    printf '%s\n' "${header_footer_separator}"
}

handle_error() {
    local exit_code="$?"

    print_ci_footer "failed"
    exit "${exit_code}"
}

trap handle_error ERR

printf '\n%s\n' "${header_footer_separator}"
print_centered "CI"
printf '%s\n\n' "${header_footer_separator}"

cd "${workspace_root}"

total_steps=$((2 + ${#presets[@]}))
step=1

print_section "Formatting"
echo "[${step}/${total_steps}] Checking formatting..."
./.vscode/tasks/format.sh "${workspace_root}/sw" "${workspace_root}" --check

step=$((step + 1))
print_section "Lint and build"
echo "[${step}/${total_steps}] Running configuration, build, and lint checks..."
./.vscode/tasks/lint.sh "${workspace_root}" "${presets[@]}"

for preset in "${presets[@]}"; do
    step=$((step + 1))
    print_section "CTest ${preset}"
    echo "[${step}/${total_steps}] Running CTest..."
    ctest --test-dir "build/${preset}" --output-on-failure \
        --test-output-size-passed 0 \
        --test-output-size-failed 0
done

print_ci_footer "passed"
