# Embedded C coding guidelines

## Glossary

| Term | Definition |
|---|---|
| Module | Set of `.c` and `.h` files sharing the same base name that implement a specific functionality. |
| Library | One or more related modules grouped under a common prefix or functional domain, representing a higher-level functional abstraction and built as a static library (`.a`). |
| Scope | Visibility of a symbol that defines where it can be accessed within the program. |
| Global scope | Accessible from anywhere in the project. |
| Module scope | Accessible only within the defining `.c` file. |
| Function scope | Accessible only within the defining function. |

## Code review criteria

### Keyword usage
---

Module-scope variables and functions shall be declared with the `static` keyword.

---

The use of the `volatile` keyword is only allowed for the following use cases:

1. **hardware_interaction**: The variable represents a memory-mapped hardware register whose value can change independently of the program flow.
2. **asynchronous_interaction**: The variable can be accessed or modified asynchronously with respect to the normal program flow (e.g. by an interrupt service routine or a signal handler).
3. **context_interaction**: The variable can be accessed or modified by multiple execution contexts that may preempt each other (e.g. a variable shared between RTOS or OS tasks).
4. **observability**: The variable shall remain visible to external debugging, tracing, or monitoring tools.

Variables declared with the `volatile` keyword shall include in their documentation a justification of its use, explicitly referencing one of the valid use cases listed above. This justification shall use the tag `volatile_use:` as shown below:

```c
/**
 * @brief ...
 *  
 * volatile_use: hardware_interaction
 */
volatile uint8_t var1;

/**
 * @brief ...
 *  
 * volatile_use: asynchronous_interaction
 */
volatile uint8_t var2;

/**
 * @brief ...
 */
typedef struct
{
    volatile uint32_t struct_member_1; /**< Attribute description ...
                                            volatile_use: context_interaction */
    volatile uint32_t struct_member_2; /**< Attribute description ...
                                            volatile_use: observability */
} struct_t;
```

---

The use of the `const` keyword is only allowed for the following use cases:

1. **ROM placement**: For global or module-scope objects intended to be stored in read-only memory.
2. **Pass-by-reference input parameter protection**: For function parameters that point to user-provided data which shall not be modified by the function, as specified in the function documentation (i.e. read-only parameters).
3. **Return type protection**: For function return types that return a pointer to data which shall not be modified by the caller (i.e. read-only data).
4. **Typedef declarations**: For typedef declarations used to define types representing read-only data or pointers to data which shall not be modified.

Unlike `volatile`, the use of `const` does not require a dedicated justification, as its purpose is typically clear from the declaration and the associated API documentation.

When `const` is used with pointer types to protect the data referenced by the pointer, the `const` qualifier shall be placed before the base type (e.g. `const uint8_t * buffer`) to ensure a consistent declaration style across the codebase.

The use of `const` to protect pass-by-value parameters or ordinary local variables is **not permitted**. This restriction is not due to any negative functional effects, but rather to maintain consistency across the codebase. Allowing `const` in these cases would make its usage arbitrary or require applying it systematically to nearly all local variables, which would reduce clarity and add unnecessary verbosity without providing actual protection or benefit. 

Discarding a `const` qualifier by typecast is normally not permitted. However, it is allowed within framework or library internal code when const is used only to enforce a read-only contract in the public API and the underlying object is not actually defined as const and not located in read-only memory. In such cases, the cast shall be explicit and accompanied by the following comment:

```c
// Typecast to discard const qualifier.
```

An example of correct `const` usage is shown below:

```c

/**
 * @brief Lookup table stored in ROM.
 */
const uint16_t lookupTable[256] = { /* ... */ };

/**
 * @brief Calculates the checksum of a data buffer.
 *
 * @param[in] data_in Pointer to input data.
 * @param[in] length Number of bytes to process.
 * @param[out] checksum Pointer to store the calculated checksum.
 */
void calculateChecksum(const uint8_t * data_in, 
                       uint16_t length, 
                       uint16_t * checksum);

void internal_update(const MyObject * obj)
{
    MyObject * mutable_obj;

    mutable_obj = (MyObject *)obj; // Typecast to discard const qualifier.
    mutable_obj->internal_state = 1;
}
```

---
### Include usage
---

Implementation files (`.c`) shall include their corresponding module header (`.h`) whenever such a header exists, to ensure consistency between the public interface and its implementation.

---

Each file shall include only the headers it directly depends on. Dependencies shall never rely on transitive includes (headers included indirectly through other headers).

---
### Naming conventions
---

Global variables and functions shall include a module-specific prefix followed by an underscore and the variable or function name, using lowerCamelCase format. 

Optionally, a library prefix (as short as possible) followed by an underscore may precede the module-specific prefix when needed to indicate the library of origin.

Examples: `libName_moduleName_variableName`, `libName_moduleName_functionName`, `moduleName_variableName`, `moduleName_functionName`

---

Global typedefs shall include a module-specific prefix followed by an underscore and the typedef name, ending with the suffix `_t`, using lowerCamelCase format.

Optionally, a library prefix (as short as possible) followed by an underscore may precede the module-specific prefix when needed to indicate the library of origin.

Examples: `libName_moduleName_typeName_t`, `moduleName_typeName_t`

---

Global enumerators shall include a module-specific prefix followed by the enumerator name, using UPPER_CASE format. 

Optionally, a library prefix (as short as possible) may precede the module-specific prefix when needed to indicate the library of origin.

Examples: `LIB_NAME_MODULE_NAME_ENUMERATOR_NAME`, `MODULE_NAME_ENUMERATOR_NAME`

---

Global macros shall include a module-specific prefix followed by the macro name, using UPPER_CASE format. 

Optionally, a library prefix (as short as possible) may precede the module-specific prefix when needed to indicate the library of origin. 

Examples:  `LIB_NAME_MODULE_NAME_MACRO_NAME`, `MODULE_NAME_MACRO_NAME`

---

Module-scope variables and functions shall use lowerCamelCase format. 

Examples: `variableName`, `functionName`

---

Module-scope typedefs shall end with the suffix `_t`, using lowerCamelCase format.

Example: `typeName_t`

---

Module-scope enumerators shall use UPPER_CASE format.

Example: `ENUMERATOR_NAME`

---

Module-scope macros shall use UPPER_CASE format.

Example: `MACRO_NAME`

---

Function-scope variables shall use the snake_case naming convention.  

Example: `variable_name`

---

Struct members shall use snake_case format.

Example: `struct_member_name`

---
### Implementation patterns
---

The following implementation pattern shall be applied whenever possible.

#### Function structure
---

The following function structure shall be followed whenever possible:

1. Declare the local variables.
2. Place the assertions and input validations that can be checked at that point.
3. Initialize the working variables.
4. Implement the main function logic.
5. Return from a single final `return` statement.

Example:

```c
uint16_t module_compute(const uint8_t * data_in, uint16_t length)
{
  uint16_t result;
  uint16_t index;

  EAF_ASSERT(data_in != NULL);
  EAF_ASSERT(length > 0U);

  result = 0U;

  for (index = 0U; index < length; ++index)
  {
    result = (uint16_t)(result + data_in[index]);
  }

  return result;
}
```

---
### File templates
---

The following template shall be used for `.c` files:

```c
/*******************************************************************************
 * @brief <Brief description of the file purpose>
 *
 * <Detailed description if needed>
 *
 * @copyright
 * Copyright (c) <Year> <Author/Organization>.
 * See the LICENSE file of this project for license details.
 * This notice shall be retained in all copies or substantial portions
 * of the software.
 *
 * @note
 * This note is optional. Use only if the file is a derivative work.
 * Example:
 * This file is a derivative work based on:
 * <ProjectName> (c) <Author/Organization>.
 *
 * @warning
 * This software is provided "as is", without any express or implied warranty.
 * The user assumes all responsibility for its use and any consequences.
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/
```

---

The following template shall be used for `.h` files

```c
/*******************************************************************************
 * @brief <Brief description of the file purpose>
 *
 * <Detailed description if needed>
 *
 * @copyright
 * Copyright (c) <Year> <Author/Organization>.
 * See the LICENSE file of this project for license details.
 * This notice shall be retained in all copies or substantial portions
 * of the software.
 *
 * @note
 * This note is optional. Use only if the file is a derivative work.
 * Example:
 * This file is a derivative work based on:
 * <ProjectName> (c) <Author/Organization>.
 *
 * @warning
 * This software is provided "as is", without any express or implied warranty.
 * The user assumes all responsibility for its use and any consequences.
 ******************************************************************************/

#ifndef MODULE_H
#define MODULE_H

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

#endif /* MODULE_H */
```

---

The following template shall be used for `.S` files

```asm
/*******************************************************************************
 * @brief <Brief description of the file purpose>
 * 
 * <Detailed description if needed>
 *
 * @copyright
 * Copyright (c) <Year> <Author/Organization>.
 * See the LICENSE file of this project for license details.
 * This notice shall be retained in all copies or substantial portions
 * of the software.
 *
 * @note
 * This note is optional. Use only if the file is a derivative work.
 * Example:
 * This file is a derivative work based on:
 * <ProjectName> (c) <Author/Organization>.
 *
 * @warning
 * This software is provided "as is", without any express or implied warranty.
 * The user assumes all responsibility for its use and any consequences.
 ******************************************************************************/

/*******************************************************************************
 * ASSEMBLER SETTINGS
 ******************************************************************************/

/*******************************************************************************
 * EXTERNAL SYMBOLS
 ******************************************************************************/

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * DATA SECTION
 ******************************************************************************/

/*******************************************************************************
 * CODE SECTION
 ******************************************************************************/
```

---
### Documentation

#### Source code documentation
---

All code elements shall be documented using **Doxygen** comments.

The following Doxygen tags are allowed: `@brief`, `@param`, `@return`, `@note`, `@todo`, `@ref`, `@c`, `@par`, `@anchor`, `@warning`

When a documentation fragment needs to be referenced, the following format shall be used:

```c
/**
 * @anchor <anchor_name>
 * @par <Visible title>
 *
 * ...
 */
```

---

Multi-line documentation blocks shall use the following Doxygen format:

```c
/**
 * @brief ...
 *
 * ... @ref ...
 *
 * @note ...
 *
 * @todo ...
 *
 * @param[in] ...
 * @param[out] ...
 * @param[in,out] ...
 * @return ...
 */
```

---

Single-line documentation, used to document `struct` or `enum` members, shall use the following Doxygen format:

```c
//!< ...
```

---

Multi-line documentation, used to document `struct` or `enum` members, shall use the following Doxygen format:

```c
/**< ...
     ... */
```
                                            
---

Single-line code comments shall use the following format:

```c
// ...
```

---

Multi-line code comments shall use the following format:

```c
/**
 * ...
 * ...
 */
```

---
##### Library/module documentation
---

The following template shall be used to document libraries and modules in a separate Markdown (.md) file.

```
# <Library/module name> overview

<High-level description of the purpose, intent and scope of the library/module>.

# Glossary

| Term | Definition |
|---|---|
| <Term used in the library/module> | <Definition of the term in the context of the library/module>. |

# Usage example

<Minimal example demonstrating how the library/module is intended to be used externally in practice>.
```

The Markdown documentation file shall have the same base name as the library/module, only changing the extension (e.g., my_lib.a -> my_lib.md, my_module.c -> my_module.md).

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

The Markdown documentation file shall have the same base name as the test source file, only changing the extension (e.g., test_example.c --> test_example.md).

---
## Tool-based review criteria

### GCC compiler
---

All code shall compile with **GCC** without warnings, using the following flags (or stricter):

```
-Wall -Wextra -Werror
```

---

### Clang-tidy
---

All C files shall be statically analyzed using `clang-tidy` with the [.clang-tidy](../../../../.clang-tidy) configuration file.

---
### Clang-format
---

All C files shall be formatted using `clang-format` with the [.clang-format](../../../../.clang-format) configuration file.

`// clang-format off` and `// clang-format on` shall only be used when strictly necessary to preserve readability in cases such as table initializations or similar structured layouts. Their use shall be minimized.

---
### CMake-format
---

All CMake files shall be formatted using `cmake-format` with the [.cmake-format.yaml](../../../../.cmake-format.yaml) configuration file.

---
### Assembly format
---

Assembly source files shall use the .S extension (uppercase) to ensure the preprocessor is applied.

Assembly (`.S`) files shall be formatted using the [asm_format.py](../../../../tools/asm_format/asm_format.py) formatter.

---
