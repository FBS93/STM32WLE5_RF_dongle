"""Black-box tests for Docgen's Doxygen API output."""

from conftest import run_docgen


def test_generates_api_pages_only_for_selected_supported_sources(tmp_path):
  repository_root = tmp_path / "repository"
  repository_root.mkdir()
  (repository_root / ".gitignore").write_text("ignored.h\n", encoding="utf-8")
  (repository_root / "selected.h").write_text(
    "/** @brief Selected API. */\nvoid selected(void);\n", encoding="utf-8"
  )
  (repository_root / "selected.c").write_text(
    "void selected_implementation(void) {}\n", encoding="utf-8"
  )
  (repository_root / "ignored.h").write_text(
    "void ignored(void);\n", encoding="utf-8"
  )
  (repository_root / "startup.S").write_text(
    ".global startup\nstartup:\n  bx lr\n", encoding="utf-8"
  )
  output_path = repository_root / "report"

  result = run_docgen(repository_root, output_path)

  assert result.returncode == 0, result.stderr
  index = (output_path / "index.html").read_text(encoding="utf-8")
  assert "selected [API: selected.h]" in index
  assert "selected [API: selected.c]" in index
  assert "ignored [API: ignored.h]" not in index
  assert "startup [API: startup.S]" not in index
  assert (output_path / "doxygen/html/index.html").is_file()
