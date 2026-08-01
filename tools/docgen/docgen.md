# Documentation generator overview

`docgen.py` generates static, navigable HTML documentation from a selected repository tree:
- UTF-8 Markdown is rendered, other UTF-8 files are shown as source, and non-text files are copied as assets.
- When C or C++ source files are present, Docgen runs Doxygen to generate API pages by default.
- Attached test results are added as individual test result pages.
- By default, it ignores paths matched by `.gitignore`, the `.git` directory, and the generated output directory.

Run the Docgen test suite with:

```bash
pytest tools/docgen/test
```

# Glossary

| Term | Definition |
|---|---|
| Static documentation | Generated HTML pages and copied assets that preserve selected repository-relative paths. |
| API page | Docgen page that displays a native Doxygen file page. |

# Usage example

Options:
- `--root <path>`: Repository root; defaults to the current directory.
- `--output <path>`: Output directory; defaults to `build/docgen`. Relative paths are resolved below `--root`; the directory must be a child of `--root` and is replaced on generation.
- `--title <text>`: Project title; defaults to the resolved root folder basename.
- `--exclude <path>`: Root-relative path or basename to exclude; may be repeated. A matching directory excludes its descendants, and a basename matches every entry with that name.
- `--include-ignored`: Include paths ignored by `.gitignore`.
- `--no-api`: Do not run Doxygen or generate API pages.
- `--strict`: Fail on Doxygen, local-link, or symlink warnings.
- `--test-result <label> <path>`: Add a UTF-8 test log page; may be repeated.

Generate repository documentation with test-result pages:

```bash
python tools/docgen/docgen.py \
  --root . --output build/docgen --title "Embedded Workbench" \
  --test-result "Unit tests" build/unit-tests.log \
  --test-result "Integration tests" build/integration-tests.log
```
