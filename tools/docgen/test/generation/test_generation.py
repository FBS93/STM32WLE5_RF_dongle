"""Black-box tests for Docgen's generated documentation."""

from conftest import run_docgen


def test_generates_markdown_text_assets_and_resolved_local_links(tmp_path):
  repository_root = tmp_path / "repository"
  repository_root.mkdir()
  (repository_root / "README.mdown").write_text(
    "# Overview\n\n[Jump](#overview)\n\n"
    "[Source](source.txt?download=1#section)\n\n"
    "![Asset](image.bin?width=20#preview)\n",
    encoding="utf-8",
  )
  (repository_root / "guide.markdown").write_text("# Guide\n", encoding="utf-8")
  (repository_root / "source.txt").write_text(
    "<script>not executable</script>\n", encoding="utf-8"
  )
  (repository_root / "image.bin").write_bytes(b"\x00binary")
  output_path = repository_root / "report"

  result = run_docgen(repository_root, output_path, "--no-api")

  assert result.returncode == 0, result.stderr
  assert "Overview" in (output_path / "index.html").read_text(encoding="utf-8")
  markdown_page = (output_path / "files/README.mdown.html").read_text(
    encoding="utf-8"
  )
  assert "Overview" in markdown_page
  assert "Source" in markdown_page
  assert (output_path / "files/source.txt.html").is_file()
  assert (output_path / "assets/image.bin").read_bytes() == b"\x00binary"
  assert "Guide" in (output_path / "files/guide.markdown.html").read_text(
    encoding="utf-8"
  )
  assert "&lt;script&gt;not executable&lt;/script&gt;" in (
    output_path / "files/source.txt.html"
  ).read_text(encoding="utf-8")


def test_reports_local_link_warnings_and_strict_mode_rejects_them(tmp_path):
  repository_root = tmp_path / "repository"
  repository_root.mkdir()
  (repository_root / "README.md").write_text(
    "[Missing](missing.txt)\n", encoding="utf-8"
  )
  output_path = repository_root / "report"

  result = run_docgen(repository_root, output_path, "--no-api", "--strict")

  assert result.returncode == 1
  assert "strict mode rejects documentation warnings" in result.stderr
  assert "README.md: local link target not mirrored: missing.txt" in (
    output_path / "warnings.txt"
  ).read_text(encoding="utf-8")
  index = (output_path / "index.html").read_text(encoding="utf-8")
  assert "Generation report" in index
  assert "README.md: local link target not mirrored: missing.txt" in index


def test_generation_report_shows_no_warnings(tmp_path):
  repository_root = tmp_path / "repository"
  repository_root.mkdir()
  (repository_root / "README.md").write_text("# Overview\n", encoding="utf-8")
  output_path = repository_root / "report"

  result = run_docgen(repository_root, output_path, "--no-api")

  assert result.returncode == 0, result.stderr
  index_text = (output_path / "index.html").read_text(encoding="utf-8")
  assert "Overview" in index_text
  assert "Generation report" in index_text
  assert "files processed; 0 API associations accepted." in index_text
  assert "Warnings" in index_text
  assert "None" in index_text


def test_generates_directory_pages_and_resolves_directory_links(tmp_path):
  repository_root = tmp_path / "repository"
  guides = repository_root / "guides"
  nested = guides / "nested"
  nested.mkdir(parents=True)
  (repository_root / "README.md").write_text(
    "[Guides](guides/?view=all#start)\n", encoding="utf-8"
  )
  (guides / "intro.md").write_text("# Introduction\n", encoding="utf-8")
  (nested / "detail.txt").write_text("detail\n", encoding="utf-8")
  output_path = repository_root / "report"

  result = run_docgen(repository_root, output_path, "--no-api", "--strict")

  assert result.returncode == 0, result.stderr
  directory_page = output_path / "files/guides/index.html"
  assert "Introduction" in (
    output_path / "files/guides/intro.md.html"
  ).read_text(encoding="utf-8")
  assert "detail" in (
    output_path / "files/guides/nested/detail.txt.html"
  ).read_text(encoding="utf-8")
  assert "Guides" in (output_path / "files/README.md.html").read_text(
    encoding="utf-8"
  )
  assert directory_page.is_file()
  assert "local link target not mirrored: guides/" not in (
    output_path / "warnings.txt"
  ).read_text(encoding="utf-8")


def test_generates_individual_escaped_test_result_pages(tmp_path):
  repository_root = tmp_path / "repository"
  repository_root.mkdir()
  (repository_root / "README.md").write_text("# Overview\n", encoding="utf-8")
  first_log = tmp_path / "first.log"
  second_log = tmp_path / "second.log"
  first_log.write_text("<failure>& details</failure>\n", encoding="utf-8")
  second_log.write_text("second\n", encoding="utf-8")
  output_path = repository_root / "report"

  result = run_docgen(
    repository_root,
    output_path,
    "--no-api",
    "--test-result",
    "Unit <tests>",
    str(first_log),
    "--test-result",
    "Integration",
    str(second_log),
  )

  assert result.returncode == 0, result.stderr
  first_page = (output_path / "test-results/1.html").read_text(encoding="utf-8")
  second_page = (output_path / "test-results/2.html").read_text(
    encoding="utf-8"
  )
  assert "Unit &lt;tests&gt;" in first_page
  assert "&lt;failure&gt;&amp; details&lt;/failure&gt;" in first_page
  assert "Integration" in second_page
  assert not (output_path / "test-results.html").exists()
