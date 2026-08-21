# ==============================================================================
# @brief Black-box tests for the PDF-to-Markdown command-line interface.
#
# The tests execute the production script in separate processes and observe
# only command-line results and filesystem effects.
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
# Standard library imports
# ------------------------------------------------------------------------------
import os
import subprocess
import sys
from pathlib import Path

# ------------------------------------------------------------------------------
# External library imports
# ------------------------------------------------------------------------------
import pytest

# ------------------------------------------------------------------------------
# Project-specific imports
# ------------------------------------------------------------------------------

# ==============================================================================
# CONSTANTS
# ==============================================================================

##
# @brief Path to the production PDF-to-Markdown command-line script.
##
SCRIPT_PATH = Path(__file__).parents[1] / "pdf_to_markdown.py"

# ==============================================================================
# CLASSES
# ==============================================================================

# ==============================================================================
# FUNCTIONS
# ==============================================================================


##
# @brief Create a deterministic Docling substitute for command-line tests.
#
# The substitute is exposed to child processes through @c PYTHONPATH. It
# records conversion calls and supports controlled output and failure through
# environment variables.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @return Tuple containing the child-process environment and call-log path.
##
@pytest.fixture
def fake_docling(tmp_path):
  package_root = tmp_path / "fake-packages"
  package_files = {
    "docling/__init__.py": "",
    "docling/datamodel/__init__.py": "",
    "docling/datamodel/base_models.py": (
      'class ConversionStatus:\n  SUCCESS = "success"\n\n'
      'class InputFormat:\n  PDF = "pdf"\n'
    ),
    "docling/datamodel/pipeline_options.py": (
      "class PdfPipelineOptions:\n  pass\n\n"
      "class RapidOcrOptions:\n"
      "  def __init__(self, **options):\n"
      "    self.options = options\n"
    ),
    "docling/document_converter.py": (
      "import os\n"
      "from pathlib import Path\n\n"
      "class PdfFormatOption:\n"
      "  def __init__(self, pipeline_options):\n"
      "    self.pipeline_options = pipeline_options\n\n"
      "class FakeDocument:\n"
      "  def save_as_markdown(self, filename, image_mode):\n"
      '    content = os.environ.get("FAKE_DOCLING_MARKDOWN", '
      '"# Converted\\n")\n'
      '    filename.write_text(content, encoding="utf-8")\n'
      '    if os.environ.get("FAKE_DOCLING_FAIL") == "1":\n'
      '      raise RuntimeError("simulated Docling failure")\n\n'
      "class FakeResult:\n"
      '  status = "success"\n'
      "  document = FakeDocument()\n\n"
      "class DocumentConverter:\n"
      "  def __init__(self, allowed_formats, format_options):\n"
      "    pass\n\n"
      "  def convert(self, source, raises_on_error):\n"
      '    call_log = os.environ.get("FAKE_DOCLING_CALL_LOG")\n'
      "    if call_log:\n"
      '      with Path(call_log).open("a", encoding="utf-8") as log:\n'
      '        log.write(f"{source}\\n")\n'
      "    return FakeResult()\n"
    ),
    "docling_core/__init__.py": "",
    "docling_core/types/__init__.py": "",
    "docling_core/types/doc.py": (
      'class ImageRefMode:\n  EMBEDDED = "embedded"\n'
    ),
    "docling-9.9.9.dist-info/METADATA": (
      "Metadata-Version: 2.1\nName: docling\nVersion: 9.9.9\n"
    ),
  }
  for relative_path, content in package_files.items():
    file_path = package_root / relative_path
    file_path.parent.mkdir(parents=True, exist_ok=True)
    file_path.write_text(content, encoding="utf-8")

  call_log = tmp_path / "docling-calls.log"
  environment = os.environ.copy()
  python_path = environment.get("PYTHONPATH")
  environment["PYTHONPATH"] = os.pathsep.join(
    path for path in (str(package_root), python_path) if path
  )
  environment["FAKE_DOCLING_CALL_LOG"] = str(call_log)
  return environment, call_log


##
# @brief Write a minimal PDF-like input file accepted by the command.
#
# @param[out] file_path Destination path for the input file.
# @param[in] payload Content appended after the PDF header.
##
def write_pdf(file_path, payload=b"content"):
  file_path.write_bytes(b"%PDF-1.7\n" + payload)


##
# @brief Run the PDF-to-Markdown command in a separate process.
#
# @param[in] input_path Input PDF path passed to the command.
# @param[in] output_path Output Markdown path passed to the command.
# @param[in] environment Environment supplied to the child process.
# @param[in] extra_arguments Additional command-line arguments.
# @return Completed child-process result.
##
def run_cli(input_path, output_path, environment, *extra_arguments):
  return subprocess.run(
    [
      sys.executable,
      str(SCRIPT_PATH),
      "--input",
      str(input_path),
      "--output",
      str(output_path),
      *extra_arguments,
    ],
    check=False,
    capture_output=True,
    text=True,
    env=environment,
  )


##
# @brief Count recorded calls to the deterministic Docling substitute.
#
# @param[in] call_log Path to the substitute call log.
# @return Number of recorded conversion calls.
##
def call_count(call_log):
  if not call_log.exists():
    return 0
  return len(call_log.read_text(encoding="utf-8").splitlines())


##
# @brief Verify conversion to an explicit path and unchanged-output reuse.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_converts_to_explicit_output_then_reuses_cache(tmp_path, fake_docling):
  environment, call_log = fake_docling
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "nested" / "exact-name.md"
  write_pdf(input_path)
  environment["FAKE_DOCLING_MARKDOWN"] = "# Converted\r\n"

  converted = run_cli(input_path, output_path, environment)
  environment["FAKE_DOCLING_FAIL"] = "1"
  cached = run_cli(input_path, output_path, environment)

  assert converted.returncode == 0, converted.stderr
  assert "Markdown written" in converted.stdout
  assert output_path.is_file()
  assert output_path.read_bytes().endswith(b"# Converted\n")
  assert not (output_path.parent / "source.md").exists()
  assert cached.returncode == 0, cached.stderr
  assert "cache hit" in cached.stdout
  assert call_count(call_log) == 1


##
# @brief Verify that collisions and stale outputs require explicit replacement.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_collision_and_stale_input_require_force(tmp_path, fake_docling):
  environment, call_log = fake_docling
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path, b"first")
  output_path.write_text("manual output\n", encoding="utf-8")

  collision = run_cli(input_path, output_path, environment)
  assert collision.returncode == 1
  assert "--force" in collision.stderr
  assert output_path.read_text(encoding="utf-8") == "manual output\n"

  first_conversion = run_cli(input_path, output_path, environment, "--force")
  assert first_conversion.returncode == 0, first_conversion.stderr
  first_output = output_path.read_bytes()
  write_pdf(input_path, b"second")

  stale = run_cli(input_path, output_path, environment)
  assert stale.returncode == 1
  assert "--force" in stale.stderr
  assert output_path.read_bytes() == first_output

  environment["FAKE_DOCLING_MARKDOWN"] = "second conversion\n"
  replacement = run_cli(input_path, output_path, environment, "--force")
  assert replacement.returncode == 0, replacement.stderr
  assert output_path.read_bytes().endswith(b"second conversion\n")
  assert call_count(call_log) == 2


##
# @brief Verify rejection of invalid input and destination paths.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
# @param[in] case Invalid-path scenario identifier.
# @param[in] message Expected user-facing error fragment.
##
@pytest.mark.parametrize(
  ("case", "message"),
  [
    ("missing", "does not exist"),
    ("wrong_extension", "must end in .pdf"),
    ("invalid_header", "PDF header"),
    ("wrong_output_extension", "must end in .md"),
  ],
)
def test_rejects_invalid_inputs_and_destinations(
  tmp_path, fake_docling, case, message
):
  environment, call_log = fake_docling
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"

  if case == "missing":
    pass
  elif case == "wrong_extension":
    input_path = tmp_path / "source.txt"
    write_pdf(input_path)
  elif case == "invalid_header":
    input_path.write_text("not a PDF\n", encoding="utf-8")
  else:
    write_pdf(input_path)
    output_path = tmp_path / "result.txt"

  result = run_cli(input_path, output_path, environment)

  assert result.returncode == 1
  assert "PDF to Markdown failed" in result.stderr
  assert message in result.stderr
  assert not output_path.exists()
  assert call_count(call_log) == 0


##
# @brief Verify that failed conversion never publishes partial Markdown.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
# @param[in] existing_output Whether a previous destination shall be preserved.
##
@pytest.mark.parametrize("existing_output", [False, True])
def test_conversion_failure_never_publishes_partial_output(
  tmp_path, fake_docling, existing_output
):
  environment, call_log = fake_docling
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)
  if existing_output:
    output_path.write_text("previous\n", encoding="utf-8")
  environment["FAKE_DOCLING_MARKDOWN"] = "partial\n"
  environment["FAKE_DOCLING_FAIL"] = "1"

  result = run_cli(input_path, output_path, environment, "--force")

  assert result.returncode == 1
  assert "PDF to Markdown failed" in result.stderr
  if existing_output:
    assert output_path.read_text(encoding="utf-8") == "previous\n"
  else:
    assert not output_path.exists()
  assert sorted(path.name for path in tmp_path.iterdir()) == sorted(
    [
      "docling-calls.log",
      "fake-packages",
      "source.pdf",
      *(["result.md"] if existing_output else []),
    ]
  )
  assert call_count(call_log) == 1
