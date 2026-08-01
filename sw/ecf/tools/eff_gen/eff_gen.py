#!/usr/bin/env python3

# ==============================================================================
# @brief Fake functions generator for Embedded Fake Framework.
#
# Generates EFF-compatible fake headers and sources from C header prototypes.
#
# Usage:
#   python3 eff_gen.py -i <input_path> -o <output_path>
#
# Parameters:
#   -i Path to input header file or directory containing headers.
#   -o Path to output directory where generated files are stored.
#
# @copyright
# Copyright (c) 2026 FBS93.
# See the LICENSE file of this project for license details.
# This notice shall be retained in all copies or substantial portions
# of the software.
#
# @note
# This file is a derivative work based on:
# fff-mock-gen (c) Amcolex.
#
# @warning
# This software is provided "as is", without any express or implied warranty.
# The user assumes all responsibility for its use and any consequences.
# ==============================================================================

# ==============================================================================
# IMPORTS
# ==============================================================================

# ------------------------------------------------------------------------------
# Standard library imports
# ------------------------------------------------------------------------------
import argparse
import glob
import re
import shutil
from pathlib import Path

# ------------------------------------------------------------------------------
# External library imports
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Project-specific imports
# ------------------------------------------------------------------------------

# ==============================================================================
# CONSTANTS
# ==============================================================================

##
# @brief Matches a full function prototype declaration terminated by @c ;.
#
# Captures the declaration prefix, function name and raw argument list after
# declarations have already been sanitized from comments and preprocessor lines.
##
FUNCTION_PROTOTYPE_PATTERN = re.compile(
  r"^(?P<return_type>.+?)"
  r"(?P<function_name>[A-Za-z_]\w*)"
  r"\s*\("
  r"(?P<arguments>.*)"
  r"\)\s*;$",
  re.DOTALL,
)

##
# @brief Matches inline function-pointer argument declarations.
#
# Captures the callback return type, pointer declarator prefix, argument name,
# and callback parameter list for declarations such as
# @c void (* callback)(uint8_t status).
##
FUNCTION_POINTER_ARGUMENT_PATTERN = re.compile(
  r"^(?P<return_type>.+?)"
  r"\(\s*(?P<pointer_prefix>\*\s*(?:(?:const|volatile)\s+)*)"
  r"(?P<argument_name>[A-Za-z_]\w*)\s*\)\s*"
  r"\((?P<parameters>.*)\)$",
  re.DOTALL,
)

##
# @brief Matches trailing array declarators in function arguments.
#
# Captures the non-array prefix and the full array suffix so the generator can
# normalize array parameters to pointer-form argument types.
##
ARRAY_SUFFIX_PATTERN = re.compile(
  r"^(?P<base>.*?)(?P<arrays>(\s*\[[^\]]*\])+)$", re.DOTALL
)

##
# @brief Matches the trailing argument identifier in a declaration.
#
# Captures the type prefix, any pointer suffix associated with the identifier,
# and the final argument name so the generator can strip names from prototypes.
##
TRAILING_ARGUMENT_NAME_PATTERN = re.compile(
  r"^(?P<type_prefix>.+?)"
  r"(?P<pointer_suffix>(?:\s*\*\s*(?:(?:const|volatile)\s+)*)*)"
  r"(?P<argument_name>[A-Za-z_]\w*)$",
  re.DOTALL,
)

##
# @brief Declaration specifiers removed from function return-type prefixes.
##
LEADING_DECLARATION_SPECIFIERS = (
  "extern",
  "static",
  "inline",
  "__inline",
  "__inline__",
)

##
# @brief Matches raw function-pointer types without an identifier.
#
# Used after argument parsing to detect types that must be rewritten through a
# typedef alias before being passed into EFF variadic macros.
##
FUNCTION_POINTER_TYPE_PATTERN = re.compile(
  r"^(?P<return_type>.+?)"
  r"\(\s*(?P<pointer_prefix>\*\s*(?:(?:const|volatile)\s+)*)\)\s*"
  r"\((?P<parameters>.*)\)$",
  re.DOTALL,
)

# ==============================================================================
# CLASSES
# ==============================================================================

# ==============================================================================
# FUNCTIONS
# ==============================================================================


##
# @brief Removes C and C++ comments from a source string.
#
# @param[in] source_text Header source text.
# @return Source text without comments.
##
def strip_comments(source_text):
  source_text = re.sub(r"/\*.*?\*/", "", source_text, flags=re.DOTALL)
  source_text = re.sub(r"//.*", "", source_text)
  return source_text


##
# @brief Removes preprocessor directives from a source string.
#
# @param[in] source_text Header source text.
# @return Source text without preprocessor directives.
##
def strip_preprocessor_directives(source_text):
  filtered_lines = []
  skip_continuation = False

  for line in source_text.splitlines():
    stripped_line = line.lstrip()

    if skip_continuation:
      if not line.rstrip().endswith("\\"):
        skip_continuation = False
      continue

    if stripped_line.startswith("#"):
      if line.rstrip().endswith("\\"):
        skip_continuation = True
      continue

    filtered_lines.append(line)

  return "\n".join(filtered_lines)


##
# @brief Collapses repeated whitespace to a single space.
#
# @param[in] text Input text.
# @return Normalized text.
##
def normalize_whitespace(text):
  return " ".join(text.strip().split())


##
# @brief Removes storage and inline specifiers from a declaration prefix.
#
# @param[in] return_type Declaration prefix before the function name.
# @return Normalized return type.
##
def normalize_return_type(return_type):
  normalized_return_type = normalize_whitespace(return_type)

  stripped_specifier = True
  while stripped_specifier:
    stripped_specifier = False
    for specifier in LEADING_DECLARATION_SPECIFIERS:
      prefix = f"{specifier} "
      if normalized_return_type.startswith(prefix):
        normalized_return_type = normalized_return_type[len(prefix) :].strip()
        stripped_specifier = True

  return normalized_return_type


##
# @brief Splits a string using a delimiter only at top level.
#
# @param[in] text Input text.
# @param[in] delimiter Single-character delimiter.
# @return List of split elements.
##
def split_top_level(text, delimiter):
  parts = []
  current_part = []
  paren_depth = 0
  bracket_depth = 0

  for character in text:
    if character == "(":
      paren_depth += 1
    elif character == ")":
      paren_depth -= 1
    elif character == "[":
      bracket_depth += 1
    elif character == "]":
      bracket_depth -= 1

    if character == delimiter and paren_depth == 0 and bracket_depth == 0:
      parts.append("".join(current_part).strip())
      current_part = []
      continue

    current_part.append(character)

  parts.append("".join(current_part).strip())
  return parts


##
# @brief Extracts candidate C declarations terminated by semicolon.
#
# @param[in] source_text Header source text.
# @return Candidate declarations.
##
def extract_declarations(source_text):
  declarations = []
  current_declaration = []
  paren_depth = 0
  brace_depth = 0
  bracket_depth = 0

  for character in source_text:
    current_declaration.append(character)

    if character == "(":
      paren_depth += 1
    elif character == ")":
      paren_depth -= 1
    elif character == "{":
      brace_depth += 1
    elif character == "}":
      brace_depth -= 1
    elif character == "[":
      bracket_depth += 1
    elif character == "]":
      bracket_depth -= 1

    if (
      character == ";"
      and paren_depth == 0
      and brace_depth == 0
      and bracket_depth == 0
    ):
      declarations.append("".join(current_declaration).strip())
      current_declaration = []

  return declarations


##
# @brief Builds a pointer type by appending pointer levels.
#
# @param[in] base_type Type before array decay.
# @param[in] pointer_levels Number of pointer levels to append.
# @return Pointer type string.
##
def build_pointer_type(base_type, pointer_levels):
  pointer_type = normalize_whitespace(base_type)

  for _ in range(pointer_levels):
    pointer_type = f"{pointer_type} *"

  return normalize_whitespace(pointer_type)


##
# @brief Extracts the argument type from a function argument declaration.
#
# @param[in] argument Full function argument declaration.
# @return Argument type only.
##
def parse_argument_type(argument):
  normalized_argument = normalize_whitespace(argument)

  if normalized_argument == "" or normalized_argument == "void":
    return "void"

  function_pointer_match = FUNCTION_POINTER_ARGUMENT_PATTERN.match(
    normalized_argument
  )
  if function_pointer_match:
    return_type = normalize_whitespace(
      function_pointer_match.group("return_type")
    )
    pointer_prefix = normalize_whitespace(
      function_pointer_match.group("pointer_prefix")
    )
    parameters = normalize_whitespace(
      function_pointer_match.group("parameters")
    )
    return f"{return_type} ({pointer_prefix})({parameters})"

  array_match = ARRAY_SUFFIX_PATTERN.match(normalized_argument)
  array_dimensions = 0
  if array_match:
    normalized_argument = normalize_whitespace(array_match.group("base"))
    array_dimensions = array_match.group("arrays").count("[")

  trailing_name_match = TRAILING_ARGUMENT_NAME_PATTERN.match(
    normalized_argument
  )
  if trailing_name_match:
    type_prefix = normalize_whitespace(trailing_name_match.group("type_prefix"))
    pointer_suffix = normalize_whitespace(
      trailing_name_match.group("pointer_suffix")
    )
    argument_type = f"{type_prefix} {pointer_suffix}".strip()
  else:
    argument_type = normalized_argument

  if array_dimensions > 0:
    return build_pointer_type(argument_type, array_dimensions)

  return normalize_whitespace(argument_type)


##
# @brief Detects whether an argument type is a raw function pointer type.
#
# @param[in] argument_type Parsed argument type.
# @return True when the argument type is a function pointer.
##
def is_function_pointer_type(argument_type):
  return FUNCTION_POINTER_TYPE_PATTERN.match(argument_type) is not None


##
# @brief Builds a typedef alias for a raw function pointer type.
#
# @param[in] function_pointer_type Raw function pointer type.
# @param[in] typedef_name Alias name to inject into the declarator.
# @return Typedef declaration string.
##
def build_function_pointer_typedef(function_pointer_type, typedef_name):
  function_pointer_match = FUNCTION_POINTER_TYPE_PATTERN.match(
    function_pointer_type
  )

  if function_pointer_match is None:
    return None

  return_type = normalize_whitespace(
    function_pointer_match.group("return_type")
  )
  pointer_prefix = normalize_whitespace(
    function_pointer_match.group("pointer_prefix")
  )
  parameters = normalize_whitespace(function_pointer_match.group("parameters"))

  return (
    f"typedef {return_type} ({pointer_prefix}{typedef_name})({parameters});"
  )


##
# @brief Builds mock argument metadata for generated macro invocations.
#
# @param[in] function_name Function name used for typedef aliases.
# @param[in] argument_types Parsed argument types.
# @return Tuple with typedef declarations and macro argument types.
##
def build_mock_argument_metadata(function_name, argument_types):
  typedef_declarations = []
  mock_argument_types = []

  for index, argument_type in enumerate(argument_types):
    if is_function_pointer_type(argument_type):
      typedef_name = f"{function_name}_arg{index}_t"
      typedef_declarations.append(
        build_function_pointer_typedef(argument_type, typedef_name)
      )
      mock_argument_types.append(typedef_name)
    else:
      mock_argument_types.append(argument_type)

  return typedef_declarations, mock_argument_types


##
# @brief Parses a C function prototype declaration.
#
# @param[in] declaration Candidate declaration.
# @return Parsed function metadata or None when unsupported.
##
def parse_function_declaration(declaration):
  normalized_declaration = normalize_whitespace(declaration)

  if normalized_declaration.startswith("typedef "):
    return None

  function_match = FUNCTION_PROTOTYPE_PATTERN.match(normalized_declaration)
  if function_match is None:
    return None

  return_type = normalize_return_type(function_match.group("return_type"))
  function_name = function_match.group("function_name")
  raw_arguments = function_match.group("arguments").strip()
  split_arguments = split_top_level(raw_arguments, ",")
  argument_types = [
    parse_argument_type(argument) for argument in split_arguments
  ]

  return {
    "return_type": return_type,
    "function_name": function_name,
    "argument_types": argument_types,
  }


##
# @brief Extracts function prototypes from a header file.
#
# @param[in] header_text Full header contents.
# @return Parsed function list.
##
def extract_functions(header_text):
  sanitized_text = strip_comments(header_text)
  sanitized_text = strip_preprocessor_directives(sanitized_text)
  declarations = extract_declarations(sanitized_text)

  functions = []
  for declaration in declarations:
    function = parse_function_declaration(declaration)
    if function is not None:
      functions.append(function)

  return functions


##
# @brief Fake functions generator.
#
# Parses command-line arguments, scans header files and generates
# corresponding mock source and header files.
##
def main():
  # get arguments
  parser = argparse.ArgumentParser()
  parser.add_argument("-i", dest="headers_path", required=True)
  parser.add_argument("-o", dest="output_directory", required=True)
  args = parser.parse_args()

  # create full paths
  input_path = Path(args.headers_path).resolve()
  output_path = Path(args.output_directory).resolve()
  print("Input path:", input_path)
  print("Output path:", output_path)

  # find all .h files
  if input_path.is_file():
    header_files = [str(input_path)]
  elif input_path.is_dir():
    header_files = glob.glob(f"{input_path}/*.h")
  else:
    print(f"Input path {input_path} is not valid.")
    exit(1)
  print(f"Found {len(header_files)} header files")

  # if files list is empty return error
  if not header_files:
    print("No files found")
    exit(1)

  # delete output directory if it exists
  if output_path.exists():
    shutil.rmtree(output_path)

  # create output directory with all parent directories
  output_path.mkdir(parents=True)

  # create inc and src directories
  inc_path = output_path / "inc"
  src_path = output_path / "src"
  inc_path.mkdir()
  src_path.mkdir()

  # open each header file
  for file in header_files:
    with open(file, encoding="utf-8") as header_file:
      functions = extract_functions(header_file.read())

    for function in functions:
      typedef_declarations, mock_argument_types = build_mock_argument_metadata(
        function["function_name"], function["argument_types"]
      )
      function["typedef_declarations"] = typedef_declarations
      function["mock_argument_types"] = mock_argument_types

    print("Generating mocks for: " + file)

    # create mock file .c and .h in inc and src
    mock_file_c = open(
      f"{output_path}/src/{Path(file).stem}_mock.c", "w", encoding="utf-8"
    )
    mock_file_h = open(
      f"{output_path}/inc/{Path(file).stem}_mock.h", "w", encoding="utf-8"
    )

    # write header to mock file .h
    mock_file_h.write("#pragma once\n")
    mock_file_h.write("#ifndef EFF_GCC_FUNCTION_ATTRIBUTES\n")
    mock_file_h.write(
      "#define EFF_GCC_FUNCTION_ATTRIBUTES __attribute__((weak))\n"
    )
    mock_file_h.write("#endif\n")
    mock_file_h.write(f'#include "{Path(file).name}"\n')
    mock_file_h.write('#include "eff.h"\n\n')

    for function in functions:
      for typedef_declaration in function["typedef_declarations"]:
        mock_file_h.write(f"{typedef_declaration}\n")

    if functions:
      mock_file_h.write("\n")

    # write mock function prototypes to mock file .h
    for function in functions:
      function_name = function["function_name"]
      mock_argument_types = ", ".join(function["mock_argument_types"])
      return_type = function["return_type"]

      # if function returns void
      if function["return_type"] == "void":
        # DECLARE_FAKE_VOID_FUNC(function_name)
        if function["mock_argument_types"][0] == "void":
          mock_file_h.write(
            f"DECLARE_FAKE_VOID_FUNC({function_name});\
          \n"
          )
        else:
          mock_file_h.write(
            f"DECLARE_FAKE_VOID_FUNC({function_name}, "
            f"{mock_argument_types});\
          \n"
          )
      else:
        # DECLARE_FAKE_VALUE_FUNC(return_type, function_name, argument_types);
        # if function argument is void
        if function["mock_argument_types"][0] == "void":
          mock_file_h.write(
            f"DECLARE_FAKE_VALUE_FUNC({return_type}, {function_name});\
          \n"
          )
        else:
          mock_file_h.write(
            f"DECLARE_FAKE_VALUE_FUNC({return_type}, "
            f"{function_name}, {mock_argument_types});\
          \n"
          )

    # write header to mock file .c
    mock_file_c.write(f'#include "{Path(file).stem}_mock.h"\n\n')

    # write mock function definitions to mock file .c
    for function in functions:
      function_name = function["function_name"]
      mock_argument_types = ", ".join(function["mock_argument_types"])
      return_type = function["return_type"]

      # if function returns void
      if function["return_type"] == "void":
        if function["mock_argument_types"][0] == "void":
          # DEFINE_FAKE_VOID_FUNC(function_name)
          mock_file_c.write(
            f"DEFINE_FAKE_VOID_FUNC({function_name});\
          \n"
          )
        else:
          # DEFINE_FAKE_VOID_FUNC(function_name, argument_types)
          mock_file_c.write(
            f"DEFINE_FAKE_VOID_FUNC({function_name}, "
            f"{mock_argument_types});\
          \n"
          )

      else:
        if function["mock_argument_types"][0] == "void":
          mock_file_c.write(
            f"DEFINE_FAKE_VALUE_FUNC({return_type}, {function_name});\
          \n"
          )
        else:
          # DEFINE_FAKE_VALUE_FUNC(return_type, function_name, argument_types);
          mock_file_c.write(
            f"DEFINE_FAKE_VALUE_FUNC({return_type}, "
            f"{function_name}, {mock_argument_types});\
          \n"
          )

    # Convert snake_case module name to lowerCamelCase
    parts = Path(file).stem.split("_")
    module_name = parts[0] + "".join(word.capitalize() for word in parts[1:])

    # Write resetAll function prototype
    mock_file_h.write(f"\nvoid EFF_{module_name}_ResetAll(void);\n")

    # Write resetAll function implementation
    mock_file_c.write(f"\nvoid EFF_{module_name}_ResetAll(void) {{\n")
    for function in functions:
      mock_file_c.write(f"  RESET_FAKE({function['function_name']});\n")
    mock_file_c.write("}\n")

    # close mock file .c and .h
    mock_file_c.close()
    mock_file_h.close()


# ==============================================================================
# SCRIPT ENTRY POINT
# ==============================================================================

if __name__ == "__main__":
  main()
