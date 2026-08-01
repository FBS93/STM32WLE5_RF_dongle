# eff_gen overview

`eff_gen` generates EFF-compatible fake headers and source files from C header function prototypes. The script is intended to cover the declaration patterns used in this repository while preserving the lightweight workflow expected by the Embedded Fake Framework integration helpers.

# Glossary

| Term | Definition |
|---|---|
|   |   |

# Usage example

The generator accepts either a single header file or a directory containing
header files. It creates the generated mock output using the standard `inc/`
and `src/` folder structure.

```bash
# Generate a mock from one header file.
python3 eff_gen.py -i path/to/module.h -o path/to/generated_mock

# Generate mocks from every header in a directory.
python3 eff_gen.py -i path/to/headers_dir -o path/to/generated_mock_dir
```
