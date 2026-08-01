#!/usr/bin/env bash
set -e

echo "🧹 Clean KiCad runtime state"

# Validate required environment variables.
: "${KICAD_CONFIG_HOME:?Missing KICAD_CONFIG_HOME}"
: "${KICAD_DOCUMENTS_HOME:?Missing KICAD_DOCUMENTS_HOME}"

config_root="${KICAD_CONFIG_HOME}"
documents_root="${KICAD_DOCUMENTS_HOME}"

if [ ! -d "${config_root}" ]; then
    echo "❌ Error: KiCad config directory not found: ${config_root}"
    exit 1
fi

if [ ! -d "${documents_root}" ]; then
    echo "❌ Error: KiCad documents directory not found: ${documents_root}"
    exit 1
fi

echo "🧹 Cleaning KiCad runtime state ..."

shopt -s dotglob nullglob
config_entries=("${config_root}"/*)
documents_entries=("${documents_root}"/*)

if [ ${#config_entries[@]} -gt 0 ]; then
    rm -rf "${config_entries[@]}"
fi

if [ ${#documents_entries[@]} -gt 0 ]; then
    rm -rf "${documents_entries[@]}"
fi

echo "✅ KiCad runtime state cleaned."
