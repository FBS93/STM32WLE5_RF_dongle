# Asm format overview

`asm_format.py` formats GNU assembler (`.S`) files according to the following rules:
- Tabs are replaced with 2 spaces.
- Leading indentation is rounded up to 2-space steps except for block-comment body lines.
- Trailing whitespaces are removed.
- Line length over 80 columns is reported as a warning.

# Glossary

| Term | Definition |
|---|---|
|   |   |

# Usage example

Arguments:
- `--check`: Check formatting without modifying files.
- `<path>`: File or directory path. Directories are searched recursively for `.S` files.

Format one assembler file:

```bash
python tools/asm_format/asm_format.py sw/src/example/src/startup.S
```

Format every assembler file under one directory:

```bash
python tools/asm_format/asm_format.py sw/
```

Report formatting errors without modifying files:

```bash
python tools/asm_format/asm_format.py --check sw/
```
