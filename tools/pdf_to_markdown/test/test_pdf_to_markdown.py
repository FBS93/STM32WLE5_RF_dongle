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
import errno
import os
import pty
import re
import select
import signal
import subprocess
import sys
import termios
import time
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

##
# @brief Match the complete progress message independent of TTY controls.
##
PROGRESS_PATTERN = re.compile(
  r"PDF to Markdown \| (?P<percentage>\d+\.\d)% "
  r"\((?P<completed>\d+)/(?P<total>\d+) pages\) \| "
  r"segment (?P<segment>\d+)/(?P<segments>\d+) "
  r"\[pages (?P<start>\d+)-(?P<end>\d+)\] \| "
  r"elapsed (?P<elapsed>\d{2,}:\d{2}:\d{2})"
)

##
# @brief Maximum duration allowed for one CLI test invocation.
##
CLI_TIMEOUT_SECONDS = 20.0

##
# @brief Maximum duration allowed to reap a timed-out CLI process.
##
PROCESS_REAP_TIMEOUT_SECONDS = 5.0

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
# supports controlled output, delay and failure through environment variables.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @return Child-process environment containing the substitute packages.
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
      "import logging\n"
      "import os\n"
      "import signal\n"
      "import time\n"
      "from pathlib import Path\n\n"
      "class PdfFormatOption:\n"
      "  def __init__(self, pipeline_options):\n"
      "    self.pipeline_options = pipeline_options\n\n"
      "class FakeDocument:\n"
      "  def __init__(self, page_range):\n"
      "    self.page_range = page_range\n\n"
      "  def save_as_markdown(self, filename, image_mode):\n"
      '    content = os.environ.get("FAKE_DOCLING_MARKDOWN")\n'
      "    large_content_bytes = os.environ.get(\n"
      '      "FAKE_DOCLING_MARKDOWN_BYTES"\n'
      "    )\n"
      "    if large_content_bytes is not None:\n"
      '      content = "x" * int(large_content_bytes) + "\\n"\n'
      "    if content is None:\n"
      '      content = "\\n".join(\n'
      '        f"Page {page}" for page in range(\n'
      "          self.page_range[0], self.page_range[1] + 1\n"
      "        )\n"
      '      ) + "\\n"\n'
      '    filename.write_text(content, encoding="utf-8")\n'
      '    if os.environ.get("FAKE_DOCLING_FAIL") == "1":\n'
      '      raise RuntimeError("simulated Docling failure")\n\n'
      "class FakeResult:\n"
      '  status = "success"\n'
      "  def __init__(self, page_range):\n"
      "    self.document = FakeDocument(page_range)\n\n"
      "class DocumentConverter:\n"
      "  def __init__(self, allowed_formats, format_options):\n"
      "    pass\n\n"
      "  def convert(self, source, raises_on_error, page_range):\n"
      '    if os.environ.get("FAKE_DOCLING_LOGS") == "1":\n'
      '      logging.getLogger("docling.fake").info("fake info")\n'
      '      logging.getLogger("docling.fake").warning("fake warning")\n'
      '      print("fake stdout")\n'
      '      print("fake stderr", file=__import__("sys").stderr)\n'
      '    if os.environ.get("FAKE_DOCLING_NATIVE_OUTPUT") == "1":\n'
      '      os.write(1, b"native stdout\\n")\n'
      '      os.write(2, b"native stderr\\n")\n'
      '    delay = float(os.environ.get("FAKE_DOCLING_DELAY_SECONDS", "0"))\n'
      '    ready_file = os.environ.get("FAKE_DOCLING_READY_FILE")\n'
      "    if ready_file:\n"
      "      Path(ready_file).touch()\n"
      "    if delay > 0:\n"
      "      time.sleep(delay)\n"
      '    fail_range = os.environ.get("FAKE_DOCLING_FAIL_RANGE")\n'
      '    if fail_range == f"{page_range[0]}-{page_range[1]}":\n'
      '      raise RuntimeError("simulated segment failure")\n'
      '    kill_range = os.environ.get("FAKE_DOCLING_SIGKILL_RANGE")\n'
      '    if kill_range == f"{page_range[0]}-{page_range[1]}":\n'
      "      signal.raise_signal(signal.SIGKILL)\n"
      "    return FakeResult(page_range)\n"
    ),
    "pypdfium2/__init__.py": (
      "import os\n\n"
      "class PdfDocument:\n"
      "  def __init__(self, source):\n"
      '    self.page_count = int(os.environ.get("FAKE_PDF_PAGES", "1"))\n\n'
      "  def __len__(self):\n"
      "    return self.page_count\n\n"
      "  def close(self):\n"
      "    pass\n"
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

  environment = os.environ.copy()
  python_path = environment.get("PYTHONPATH")
  environment["PYTHONPATH"] = os.pathsep.join(
    path for path in (str(package_root), python_path) if path
  )
  return environment


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
    timeout=CLI_TIMEOUT_SECONDS,
  )


##
# @brief Run the command with stdout and stderr attached to a pseudo-terminal.
#
# @param[in] input_path Input PDF path passed to the command.
# @param[in] output_path Output Markdown path passed to the command.
# @param[in] environment Environment supplied to the child process.
# @param[in] extra_arguments Additional command-line arguments.
# @return Tuple containing the exit code and combined terminal output bytes.
##
def run_cli_tty(input_path, output_path, environment, *extra_arguments):
  command = [
    sys.executable,
    str(SCRIPT_PATH),
    "--input",
    str(input_path),
    "--output",
    str(output_path),
    *extra_arguments,
  ]
  master_fd, slave_fd = pty.openpty()
  terminal_attributes = termios.tcgetattr(slave_fd)
  terminal_attributes[1] &= ~termios.ONLCR
  termios.tcsetattr(slave_fd, termios.TCSANOW, terminal_attributes)
  process = subprocess.Popen(
    command,
    stdout=slave_fd,
    stderr=slave_fd,
    env=environment,
    close_fds=True,
    start_new_session=True,
  )
  os.close(slave_fd)

  output = []
  deadline = time.monotonic() + CLI_TIMEOUT_SECONDS
  try:
    while True:
      remaining = deadline - time.monotonic()
      if remaining <= 0:
        raise subprocess.TimeoutExpired(command, CLI_TIMEOUT_SECONDS)
      readable, _writable, _exceptional = select.select(
        [master_fd],
        [],
        [],
        remaining,
      )
      if not readable:
        raise subprocess.TimeoutExpired(command, CLI_TIMEOUT_SECONDS)
      try:
        chunk = os.read(master_fd, 4096)
      except OSError as error:
        if error.errno == errno.EIO:
          break
        raise
      if not chunk:
        break
      output.append(chunk)
  except subprocess.TimeoutExpired:
    if process.poll() is None:
      os.killpg(process.pid, signal.SIGKILL)
    process.wait(timeout=PROCESS_REAP_TIMEOUT_SECONDS)
    raise
  finally:
    os.close(master_fd)

  return process.wait(timeout=PROCESS_REAP_TIMEOUT_SECONDS), b"".join(output)


##
# @brief Read the Markdown body after the production provenance comment.
#
# @param[in] output_path Published Markdown path.
# @return Published Markdown body bytes.
##
def read_body(output_path):
  return output_path.read_bytes().split(b"\n-->\n", 1)[1]


##
# @brief Extract progress message fields while ignoring terminal controls.
#
# @param[in] output User-visible command output.
# @return Ordered progress message field dictionaries.
##
def progress_records(output):
  return [match.groupdict() for match in PROGRESS_PATTERN.finditer(output)]


##
# @brief Return ordered, adjacent-distinct page ranges from public progress.
#
# @param[in] output User-visible command output.
# @return Ordered page ranges reported by the command.
##
def progress_ranges(output):
  ranges = []
  for record in progress_records(output):
    page_range = f"{record['start']}-{record['end']}"
    if not ranges or ranges[-1] != page_range:
      ranges.append(page_range)
  return ranges


##
# @brief Convert an HH:MM:SS duration to seconds for monotonicity checks.
#
# @param[in] duration Formatted progress duration.
# @return Duration in seconds.
##
def duration_seconds(duration):
  hours, minutes, seconds = (int(part) for part in duration.split(":"))
  return hours * 3600 + minutes * 60 + seconds


##
# @brief Verify conversion to an explicit path and unchanged-output reuse.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_converts_to_explicit_output_then_reuses_cache(tmp_path, fake_docling):
  environment = fake_docling
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


##
# @brief Verify a large cached Markdown output is reused without reconversion.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_large_cached_output_is_reused_without_reconversion(
  tmp_path, fake_docling
):
  environment = fake_docling
  environment["FAKE_DOCLING_MARKDOWN_BYTES"] = str(2 * 1024 * 1024)
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)

  converted = run_cli(input_path, output_path, environment)
  environment["FAKE_DOCLING_FAIL"] = "1"
  cached = run_cli(input_path, output_path, environment)

  assert converted.returncode == 0, converted.stderr
  assert output_path.stat().st_size > 2 * 1024 * 1024
  assert cached.returncode == 0, cached.stderr
  assert "cache hit" in cached.stdout


##
# @brief Verify that modifying the published body invalidates the cache.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_modified_cached_body_requires_replacement(tmp_path, fake_docling):
  environment = fake_docling
  environment["FAKE_DOCLING_MARKDOWN"] = "original body\n"
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)

  converted = run_cli(input_path, output_path, environment)
  assert converted.returncode == 0, converted.stderr
  original_output = output_path.read_bytes()
  modified_output = original_output.replace(
    b"original body\n",
    b"modified body\n",
    1,
  )
  output_path.write_bytes(modified_output)
  environment["FAKE_DOCLING_FAIL"] = "1"

  stale = run_cli(input_path, output_path, environment)
  assert stale.returncode == 1
  assert "cache hit" not in stale.stdout
  assert "--force" in stale.stderr
  assert output_path.read_bytes() == modified_output

  forced = run_cli(input_path, output_path, environment, "--force")
  assert forced.returncode == 1
  assert "cache hit" not in forced.stdout
  assert output_path.read_bytes() == modified_output


##
# @brief Verify SIGTERM stops conversion and removes all staging artifacts.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_sigterm_preserves_output_and_cleans_staging_files(
  tmp_path, fake_docling
):
  environment = fake_docling
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  ready_path = tmp_path / "worker-ready"
  write_pdf(input_path)
  environment["FAKE_DOCLING_DELAY_SECONDS"] = "10"
  environment["FAKE_DOCLING_READY_FILE"] = str(ready_path)

  process = subprocess.Popen(
    [
      sys.executable,
      str(SCRIPT_PATH),
      "--input",
      str(input_path),
      "--output",
      str(output_path),
    ],
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    text=True,
    env=environment,
  )
  try:
    deadline = time.monotonic() + 5
    while not ready_path.exists() and process.poll() is None:
      if time.monotonic() >= deadline:
        break
      time.sleep(0.01)
    assert ready_path.exists(), "Docling worker did not start"

    process.send_signal(signal.SIGTERM)
    stdout, stderr = process.communicate(timeout=10)
  finally:
    if process.poll() is None:
      process.kill()
      process.communicate()

  assert process.returncode == 1
  assert "interrupted by SIGTERM" in stderr
  assert "Markdown written" not in stdout
  assert not output_path.exists()
  assert not list(tmp_path.glob(".result.md.*"))


##
# @brief Verify that collisions and stale outputs require explicit replacement.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_collision_and_stale_input_require_force(tmp_path, fake_docling):
  environment = fake_docling
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
  environment = fake_docling
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
  environment = fake_docling
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
  assert not list(tmp_path.glob(".result.md.*"))


##
# @brief Verify default ten-page segmentation and ordered concatenation.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_default_ten_page_segments_preserve_order(tmp_path, fake_docling):
  environment = fake_docling
  environment["FAKE_PDF_PAGES"] = "23"
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)

  result = run_cli(input_path, output_path, environment)

  assert result.returncode == 0, result.stderr
  assert progress_ranges(result.stdout) == ["1-10", "11-20", "21-23"]
  assert read_body(output_path) == b"".join(
    f"Page {page}\n".encode("utf-8") for page in range(1, 24)
  )
  records = progress_records(result.stdout)
  assert records
  assert records[-1]["percentage"] == "100.0"
  assert records[-1]["completed"] == "23"
  assert records[-1]["total"] == "23"
  assert records[-1]["segment"] == "3"
  assert records[-1]["segments"] == "3"
  assert records[-1]["start"] == "21"
  assert records[-1]["end"] == "23"
  assert all(
    re.fullmatch(r"\d{2,}:\d{2}:\d{2}", record["elapsed"]) for record in records
  )
  assert "\r" not in result.stdout
  assert "\x1b[" not in result.stdout


##
# @brief Verify total elapsed progress refreshes during and across segments.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_tty_progress_refreshes_during_and_across_segments(
  tmp_path, fake_docling
):
  environment = fake_docling
  environment["FAKE_PDF_PAGES"] = "2"
  environment["FAKE_DOCLING_DELAY_SECONDS"] = "1.5"
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)

  returncode, terminal_output = run_cli_tty(
    input_path,
    output_path,
    environment,
    "--pages-per-segment",
    "1",
  )

  assert returncode == 0
  records = progress_records(terminal_output.decode("utf-8", errors="replace"))
  assert records
  durations = [duration_seconds(record["elapsed"]) for record in records]
  assert durations == sorted(durations)
  assert any(
    record["completed"] == "0" and record["elapsed"] != "00:00:00"
    for record in records
  )
  assert records[-1]["completed"] == "2"


##
# @brief Verify TTY progress reuses one line with terminal control codes.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_tty_output_uses_interactive_progress_line(tmp_path, fake_docling):
  environment = fake_docling
  environment["FAKE_PDF_PAGES"] = "3"
  environment["FAKE_DOCLING_NATIVE_OUTPUT"] = "1"
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "tty-result.md"
  normal_output_path = tmp_path / "normal-result.md"
  write_pdf(input_path)

  normal_result = run_cli(input_path, normal_output_path, environment)
  returncode, terminal_output = run_cli_tty(
    input_path,
    output_path,
    environment,
  )

  assert returncode == 0
  assert normal_result.returncode == 0, normal_result.stderr
  terminal_text = terminal_output.decode("utf-8", errors="replace")
  normal_records = progress_records(normal_result.stdout)
  terminal_records = progress_records(terminal_text)
  assert terminal_records
  assert normal_records
  assert progress_ranges(terminal_text) == progress_ranges(normal_result.stdout)
  assert terminal_records[-1]["completed"] == normal_records[-1]["completed"]
  assert terminal_records[-1]["total"] == normal_records[-1]["total"]
  assert all(
    re.fullmatch(r"\d{2,}:\d{2}:\d{2}", record["elapsed"])
    for record in terminal_records
  )
  assert b"\r" in terminal_output
  assert b"native stdout" not in terminal_output
  assert b"native stderr" not in terminal_output


##
# @brief Verify verbose TTY progress uses normal lines beside live logs.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_verbose_tty_output_uses_normal_progress_lines(tmp_path, fake_docling):
  environment = fake_docling
  environment["FAKE_PDF_PAGES"] = "12"
  environment["FAKE_DOCLING_LOGS"] = "1"
  environment["FAKE_DOCLING_NATIVE_OUTPUT"] = "1"
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)

  returncode, terminal_output = run_cli_tty(
    input_path,
    output_path,
    environment,
    "--verbose",
  )

  assert returncode == 0
  progress = progress_records(terminal_output.decode("utf-8", errors="replace"))
  assert progress
  assert progress[-1]["percentage"] == "100.0"
  assert progress[-1]["completed"] == "12"
  assert progress[-1]["elapsed"].count(":") == 2
  assert b"\r" not in terminal_output
  assert b"\x1b[" not in terminal_output
  assert b"INFO docling.fake: fake info" in terminal_output
  assert b"native stderr" in terminal_output


##
# @brief Verify normal execution suppresses successful Docling diagnostics.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_normal_output_hides_docling_logs(tmp_path, fake_docling):
  environment = fake_docling
  environment["FAKE_DOCLING_LOGS"] = "1"
  environment["FAKE_DOCLING_NATIVE_OUTPUT"] = "1"
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)

  result = run_cli(input_path, output_path, environment)

  assert result.returncode == 0, result.stderr
  assert "fake info" not in result.stdout
  assert "fake warning" not in result.stderr
  assert "fake stdout" not in result.stdout
  assert "fake stderr" not in result.stderr
  assert "native stdout" not in result.stdout
  assert "native stderr" not in result.stderr


##
# @brief Verify verbose execution exposes Docling diagnostics.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_verbose_output_shows_docling_logs(tmp_path, fake_docling):
  environment = fake_docling
  environment["FAKE_DOCLING_LOGS"] = "1"
  environment["FAKE_DOCLING_NATIVE_OUTPUT"] = "1"
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)

  result = run_cli(input_path, output_path, environment, "--verbose")

  assert result.returncode == 0, result.stderr
  assert "INFO docling.fake: fake info" in result.stderr
  assert "WARNING docling.fake: fake warning" in result.stderr
  assert "fake stdout" in result.stdout
  assert "fake stderr" in result.stderr
  assert "native stdout" in result.stdout
  assert "native stderr" in result.stderr


##
# @brief Verify failed conversion reports captured Docling diagnostics.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_failure_reports_captured_docling_diagnostics(tmp_path, fake_docling):
  environment = fake_docling
  environment["FAKE_DOCLING_LOGS"] = "1"
  environment["FAKE_DOCLING_NATIVE_OUTPUT"] = "1"
  environment["FAKE_DOCLING_FAIL_RANGE"] = "1-1"
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)

  result = run_cli(input_path, output_path, environment)

  assert result.returncode == 1
  assert "Docling diagnostics:" in result.stderr
  assert "fake info" in result.stderr
  assert "fake warning" in result.stderr
  assert "fake stdout" in result.stderr
  assert "fake stderr" in result.stderr
  assert "native stdout" in result.stderr
  assert "native stderr" in result.stderr


##
# @brief Verify TTY failures start the tool error after a completed line.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_tty_failure_separates_native_output_and_tool_error(
  tmp_path, fake_docling
):
  environment = fake_docling
  environment["FAKE_DOCLING_NATIVE_OUTPUT"] = "1"
  environment["FAKE_DOCLING_FAIL_RANGE"] = "1-1"
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)

  returncode, terminal_output = run_cli_tty(
    input_path,
    output_path,
    environment,
  )
  terminal_text = terminal_output.decode("utf-8", errors="replace")

  assert returncode == 1
  assert "native stderr" in terminal_text
  error_position = terminal_text.index("❌ PDF to Markdown failed")
  assert "\n" in terminal_text[:error_position]


##
# @brief Verify base segmentation and forced ranges partition every page once.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_forced_ranges_split_base_segments_without_overlap(
  tmp_path, fake_docling
):
  environment = fake_docling
  environment["FAKE_PDF_PAGES"] = "20"
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)

  result = run_cli(
    input_path,
    output_path,
    environment,
    "--pages-per-segment",
    "8",
    "--page-ranges",
    "2-3,10-12",
  )

  assert result.returncode == 0, result.stderr
  assert progress_ranges(result.stdout) == [
    "1-1",
    "2-3",
    "4-9",
    "10-12",
    "13-20",
  ]
  body = read_body(output_path).decode("utf-8")
  assert [body.count(f"Page {page}\n") for page in range(1, 21)] == [1] * 20


##
# @brief Verify forced ranges at boundaries, one-page ranges and adjacency.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_forced_range_boundaries_and_adjacent_ranges(tmp_path, fake_docling):
  environment = fake_docling
  environment["FAKE_PDF_PAGES"] = "10"
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)

  result = run_cli(
    input_path,
    output_path,
    environment,
    "--pages-per-segment",
    "4",
    "--page-ranges",
    "1-2,3-3,9-10",
  )

  assert result.returncode == 0, result.stderr
  assert progress_ranges(result.stdout) == [
    "1-2",
    "3-3",
    "4-7",
    "8-8",
    "9-10",
  ]


##
# @brief Verify a forced range larger than the base size remains atomic.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_large_forced_range_remains_one_segment(tmp_path, fake_docling):
  environment = fake_docling
  environment["FAKE_PDF_PAGES"] = "10"
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)

  result = run_cli(
    input_path,
    output_path,
    environment,
    "--pages-per-segment",
    "2",
    "--page-ranges",
    "4-9",
  )

  assert result.returncode == 0, result.stderr
  assert progress_ranges(result.stdout) == ["1-2", "3-3", "4-9", "10-10"]


##
# @brief Verify invalid base segment sizes are rejected by argparse.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
# @param[in] value Invalid pages-per-segment argument.
##
@pytest.mark.parametrize("value", ["0", "-1", "not-an-integer"])
def test_rejects_invalid_pages_per_segment(tmp_path, fake_docling, value):
  environment = fake_docling
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)

  result = run_cli(
    input_path,
    output_path,
    environment,
    "--pages-per-segment",
    value,
  )

  assert result.returncode == 2
  assert "positive integer" in result.stderr
  assert not output_path.exists()


##
# @brief Verify malformed and unsafe forced ranges are rejected.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
# @param[in] ranges Invalid forced range text.
##
@pytest.mark.parametrize(
  "ranges",
  [
    "",
    "1",
    "3-2",
    "1-2,2-3",
    "1-2,1-2",
    "3-4,1-2",
    "1-6",
    "1-2,4-3",
  ],
)
def test_rejects_invalid_forced_ranges(tmp_path, fake_docling, ranges):
  environment = fake_docling
  environment["FAKE_PDF_PAGES"] = "5"
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)

  result = run_cli(
    input_path,
    output_path,
    environment,
    "--page-ranges",
    ranges,
  )

  assert result.returncode == 1
  assert "PDF to Markdown failed" in result.stderr
  assert not output_path.exists()


##
# @brief Verify a failed forced segment preserves output and cleans temporaries.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_segment_failure_preserves_output_and_cleans_temporaries(
  tmp_path, fake_docling
):
  environment = fake_docling
  environment["FAKE_PDF_PAGES"] = "5"
  environment["FAKE_DOCLING_FAIL_RANGE"] = "3-3"
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)
  output_path.write_text("previous\n", encoding="utf-8")

  result = run_cli(
    input_path,
    output_path,
    environment,
    "--force",
    "--pages-per-segment",
    "1",
  )

  assert result.returncode == 1
  assert "Segment 3/5 (pages 3-3) failed" in result.stderr
  assert output_path.read_text(encoding="utf-8") == "previous\n"
  assert not list(tmp_path.glob(".result.md.*"))


##
# @brief Verify a killed worker reports termination and preserves output.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_killed_worker_preserves_output_and_cleans_temporaries(
  tmp_path, fake_docling
):
  environment = fake_docling
  environment["FAKE_PDF_PAGES"] = "2"
  environment["FAKE_DOCLING_SIGKILL_RANGE"] = "1-2"
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)
  output_path.write_text("previous\n", encoding="utf-8")

  result = run_cli(input_path, output_path, environment, "--force")

  assert result.returncode == 1
  assert "SIGKILL" in result.stderr
  assert "Segment 1/1 (pages 1-2) failed" in result.stderr
  assert output_path.read_text(encoding="utf-8") == "previous\n"
  assert not list(tmp_path.glob(".result.md.*"))


##
# @brief Verify segmentation changes invalidate the conversion cache.
#
# @param[in] tmp_path Pytest temporary directory fixture.
# @param[in] fake_docling Deterministic Docling environment fixture.
##
def test_segmentation_options_produce_distinct_cache_results(
  tmp_path, fake_docling
):
  environment = fake_docling
  environment["FAKE_PDF_PAGES"] = "3"
  input_path = tmp_path / "source.pdf"
  output_path = tmp_path / "result.md"
  write_pdf(input_path)

  first = run_cli(
    input_path,
    output_path,
    environment,
    "--pages-per-segment",
    "2",
  )
  assert first.returncode == 0, first.stderr
  first_output = output_path.read_bytes()

  environment["FAKE_DOCLING_FAIL"] = "1"
  changed = run_cli(
    input_path,
    output_path,
    environment,
    "--pages-per-segment",
    "1",
    "--force",
  )

  assert changed.returncode == 1
  assert "cache hit" not in changed.stdout
  assert output_path.read_bytes() == first_output
