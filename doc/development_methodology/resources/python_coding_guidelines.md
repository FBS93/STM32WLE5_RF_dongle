# Python coding guidelines

## Glossary

| Term | Definition |
|---|---|
| Module | Python source file (`.py`) implementing a specific functionality. |
| Package | Set of related Python modules grouped in a directory and sharing a common namespace. |
| Scope | Visibility of a name that defines where it can be accessed within the program. |
| Module scope | Accessible from anywhere in the defining module. |
| Class scope | Accessible within the defining class through class or instance access as applicable. |
| Function scope | Accessible only within the defining function. |

## Code review criteria

### Import usage
---

Each file shall import only the modules it directly depends on. Dependencies shall never rely on transitive imports (modules imported indirectly through other modules).

---

### Naming conventions
---

Module file names shall use lower_snake_case format.

Example: `module_name.py`

---

Package names shall use lower_snake_case format.

Example: `package_name`

---

Classes and exceptions shall use UpperCamelCase format.

Examples: `ClassName`, `CustomError`

---

Instance and class attributes shall use lower_snake_case format.

Example: `attribute_name`

---

Functions shall use lower_snake_case format.

Examples: `function_name`

---

Variables shall use lower_snake_case format.

Examples: `variable_name`

---

Constants shall use UPPER_SNAKE_CASE format.

Example: `CONSTANT_NAME`

---

### File templates
---

The following template shall be used for executable `.py` scripts (non-executable modules shall omit the shebang line and script entry point).

```python
#!/usr/bin/env python3

# ==============================================================================
# @brief <Brief description of the module purpose>.
#
# <Detailed description if needed>.
#
# @copyright
# Copyright (c) <Year> <Author/Organization>.
# See the LICENSE file of this project for license details.
# This notice shall be retained in all copies or substantial portions
# of the software.
#
# @note
# This note is optional. Use only if the file is a derivative work.
# Example:
# This file is a derivative work based on:
# <ProjectName> (c) <Author/Organization>.
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

# ------------------------------------------------------------------------------
# External library imports
# ------------------------------------------------------------------------------

# ------------------------------------------------------------------------------
# Project-specific imports
# ------------------------------------------------------------------------------

# ==============================================================================
# CONSTANTS
# ==============================================================================

# ==============================================================================
# CLASSES
# ==============================================================================

# ==============================================================================
# FUNCTIONS
# ==============================================================================

# ==============================================================================
# SCRIPT ENTRY POINT
# ==============================================================================

if __name__ == "__main__":
    pass
```

---
### Documentation

#### Source code documentation
---

All code elements shall be documented using **Doxygen** comments.

The following Doxygen tags are allowed: `@brief`, `@param`, `@return`, `@note`, `@todo`, `@ref`, `@c`, `@par`, `@anchor`, `@warning`

Multi-line documentation blocks shall use the following Doxygen format:

```python
##
# @brief ...
#
# ...
#
# @note ...
#
# @todo ...
#
# @param[in] parameter_name ...
# @param[out] output_name ...
# @param[in,out] state ...
# @return ...
##
def function_name(parameter_name):
  pass
```
---

Single-line code comments shall use the following format:

```python
# ...
```

---

Multi-line code comments shall use the following format:

```python
# ...
# ...
```

---
##### Package/module documentation
---

The following template shall be used to document packages and modules in a separate Markdown (.md) file.

```
# <Package/module name> overview

<High-level description of the purpose, intent and scope of the package/module>.

# Glossary

| Term | Definition |
|---|---|
| <Term used in the package/module> | <Definition of the term in the context of the package/module>. |

# Usage example

<Minimal example demonstrating how the package/module is intended to be used externally in practice>.
```

The Markdown documentation file shall have the same base name as the package/module, only changing the extension (e.g., my_module.py -> my_module.md, my_package/ -> my_package.md).

---
#### Test documentation
---

The following template shall be used to document tests in a separate Markdown (.md) file.

```
# <Test name> test overview
<Description of the purpose of the test>.

# Use case
<Description of the test and the use case being tested>.

# Verification scope
<Description of what is being verified in this test>.
```

The Markdown documentation file shall have the same base name as the test source file, only changing the extension (e.g., test_example.py -> test_example.md).

---
## Tool-based review criteria

### Python interpreter
---

All code shall compile with **Python** without syntax errors, using the following command:

```
python -m compileall
```

---

### Ruff
---

Python files shall be formatted using `Ruff` with the [pyproject.toml](../../../pyproject.toml) configuration file.

---
