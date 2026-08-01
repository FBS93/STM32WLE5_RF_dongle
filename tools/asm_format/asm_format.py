#!/usr/bin/env python3

# ==============================================================================
# @brief Simple formatter for GNU assembler (.S) files.
#
# @copyright
# Copyright (c) 2026 FBS93.
# See the LICENSE file of this project for license details.
# This notice shall be retained in all copies or substantial portions
# of the software.
#
# @warning
# This software is provided "as is", without any express or implied warranty.
# The user assumes all responsibility for its use and any consequences.
# ==============================================================================

# ==============================================================================
# IMPORTS
# ==============================================================================

# ------------------------------------------------------------------------------
# External imports
# ------------------------------------------------------------------------------
import re
import sys
from pathlib import Path

# ------------------------------------------------------------------------------
# Project-specific imports
# ------------------------------------------------------------------------------

# ==============================================================================
# CONSTANTS
# ==============================================================================

# Amount of spaces represented by one indentation level.
INDENT_SIZE_SPACES = 2

# Maximum accepted line length in characters.
MAX_LINE_LENGTH = 80

# ==============================================================================
# CLASSES
# ==============================================================================

# ==============================================================================
# FUNCTIONS
# ==============================================================================


##
# @brief Normalize leading indentation to INDENT_SIZE_SPACES granularity.
#
# @param[in] body Line body without end-of-line bytes.
# @return Line body with normalized leading indentation.
##
def normalize_indentation(body):
  normalized_body = body
  stripped = body.lstrip(b" ")
  starts_with_star = stripped.startswith(b"*")
  starts_with_slash_star = stripped.startswith(b"/*")
  is_comment_block_line = starts_with_star or starts_with_slash_star

  if not is_comment_block_line:
    leading_spaces = len(body) - len(body.lstrip(b" "))
    if leading_spaces > 0:
      remainder = leading_spaces % INDENT_SIZE_SPACES
      if remainder != 0:
        fixed_leading_spaces = leading_spaces + (INDENT_SIZE_SPACES - remainder)
        normalized_body = (b" " * fixed_leading_spaces) + body[leading_spaces:]

  return normalized_body


##
# @brief Split a raw line into body and end-of-line bytes.
#
# @param[in] line Raw line bytes, optionally including EOL bytes.
# @return Tuple (body, eol) preserving original EOL style.
##
def split_line_body_and_eol(line):
  body = line
  eol = b""

  if line.endswith(b"\r\n"):
    eol = b"\r\n"
    body = line[:-2]
  elif line.endswith(b"\n"):
    eol = b"\n"
    body = line[:-1]
  elif line.endswith(b"\r"):
    eol = b"\r"
    body = line[:-1]

  return body, eol


##
# @brief Apply whitespace normalization rules to a line body.
#
# @param[in] body Line body without end-of-line bytes.
# @return Normalized line body.
##
def format_line_body(body):
  # Replace each tab with two spaces.
  formatted_body = body.replace(b"\t", b"  ")

  # Remove trailing spaces/tabs from the line.
  formatted_body = re.sub(rb"[ \t]+$", b"", formatted_body)

  # Normalize leading indentation to 2-space steps.
  formatted_body = normalize_indentation(formatted_body)

  return formatted_body


##
# @brief Apply formatting rules to raw file bytes.
#
# @param[in] data Original file content as bytes.
# @return Formatted file content as bytes.
##
def format_content(data):
  formatted_lines = []

  for line in data.splitlines(keepends=True):
    body, eol = split_line_body_and_eol(line)
    formatted_body = format_line_body(body)
    formatted_lines.append(formatted_body + eol)

  return b"".join(formatted_lines)


##
# @brief Resolve input paths to a unique set of .S files.
#
# @param[in] raw_paths Input file/directory paths from command line.
# @return Sorted list of unique .S file paths.
##
def resolve_asm_files(raw_paths):
  asm_files = set()

  for raw_path in raw_paths:
    path = Path(raw_path).resolve()

    if not path.exists():
      print(f"❌ Error: path not found: {path}", flush=True)
      sys.exit(1)

    if path.is_dir():
      for asm_file in path.rglob("*.S"):
        if asm_file.is_file():
          asm_files.add(asm_file)
    elif path.is_file() and path.suffix == ".S":
      asm_files.add(path)

  return sorted(asm_files)


##
# @brief Format all resolved .S files in-place.
#
# @param[in] asm_files List of .S files to format.
# @return Total count of formatting updates.
##
def format_files(asm_files):
  changed_count = 0

  for asm_file in asm_files:
    original = asm_file.read_bytes()
    formatted = format_content(original)

    if formatted != original:
      asm_file.write_bytes(formatted)
      changed_count += 1

  return changed_count


##
# @brief Check formatting of all resolved .S files without modifying them.
#
# @param[in] asm_files List of .S files to check.
# @return List of files that would be updated by formatting.
##
def check_files(asm_files):
  changed_files = []

  for asm_file in asm_files:
    original = asm_file.read_bytes()
    formatted = format_content(original)

    if formatted != original:
      changed_files.append(asm_file)

  return changed_files


##
# @brief Collect line-length warnings from formatted content.
#
# @param[in] asm_files List of .S files to check.
# @return List of line-length warning messages.
##
def collect_line_length_warnings(asm_files):
  warnings = []

  for asm_file in asm_files:
    lines = asm_file.read_bytes().splitlines()
    for line_index, line in enumerate(lines, start=1):
      line_length = len(line.decode("utf-8", errors="replace"))
      if line_length > MAX_LINE_LENGTH:
        warnings.append(
          f"{asm_file}:{line_index}: line {line_index} exceeds "
          f"{MAX_LINE_LENGTH} columns"
        )

  return warnings


##
# @brief Main formatter workflow.
#
# Orchestrates:
# - Parse command-line input paths.
# - Resolve paths to .S files.
# - Apply in-place formatting.
##
def main():
  if len(sys.argv) < 2:
    print("Usage: asm_format.py [--check] <path> [<path> ...]", flush=True)
    sys.exit(1)

  check_only = False
  raw_paths = sys.argv[1:]

  if raw_paths[0] == "--check":
    check_only = True
    raw_paths = raw_paths[1:]

  if len(raw_paths) == 0:
    print("Usage: asm_format.py [--check] <path> [<path> ...]", flush=True)
    sys.exit(1)

  asm_files = resolve_asm_files(raw_paths)
  warnings = collect_line_length_warnings(asm_files)

  if check_only:
    changed_files = check_files(asm_files)
    print(
      f"ASM formatter: checked {len(asm_files)} files, "
      f"{len(changed_files)} require formatting.",
      flush=True,
    )
    for changed_file in changed_files:
      print(f"❌ Error: formatting required: {changed_file}", flush=True)
    for warning in warnings:
      print(f"Warning: {warning}", flush=True)
    if len(changed_files) > 0:
      sys.exit(1)
    return

  changed_count = format_files(asm_files)

  print(
    f"ASM formatter: processed {len(asm_files)} files, "
    f"updated {changed_count}.",
    flush=True,
  )
  for warning in warnings:
    print(f"Warning: {warning}", flush=True)


# ==============================================================================
# SCRIPT ENTRY POINT
# ==============================================================================

if __name__ == "__main__":
  main()
