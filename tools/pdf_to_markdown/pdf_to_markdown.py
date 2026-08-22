#!/usr/bin/env python3

# ==============================================================================
# @brief Convert one local PDF file to one versioned Markdown file with Docling.
#
# The generated Markdown carries deterministic provenance metadata used to skip
# unchanged conversions. Output is staged beside the destination and published
# atomically only after conversion and integrity checks complete.
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
import argparse
import contextlib
import functools
import hashlib
import importlib
import importlib.metadata
import io
import json
import logging
import os
import re
import shutil
import signal
import subprocess
import sys
import tempfile
import threading
import time
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
# @brief Semantic version of this conversion tool and its output contract.
##
TOOL_VERSION = "1.2.0"

##
# @brief Version of the segment construction and concatenation contract.
##
SEGMENTATION_VERSION = 1

##
# @brief Canonical options that affect generated Markdown content.
##
CONVERSION_OPTIONS = {
  "format": "pdf",
  "generate_page_images": False,
  "generate_picture_images": True,
  "image_mode": "embedded",
  "images_scale": 1.0,
  "layout_batch_size": 1,
  "line_ending": "lf",
  "ocr_backend": "onnxruntime",
  "ocr_batch_size": 1,
  "ocr_engine": "rapidocr",
  "ocr_language": "english",
  "queue_max_size": 4,
  "table_batch_size": 1,
}

##
# @brief Logger namespaces owned by Docling and its conversion extensions.
##
DOCLING_LOGGER_NAMES = (
  "docling",
  "docling_core",
  "docling_parse",
  "docling_ibm_models",
)

##
# @brief Prefix identifying the provenance comment in generated Markdown.
##
METADATA_PREFIX = b"<!-- pdf-to-markdown provenance\n"

##
# @brief Suffix separating provenance metadata from generated Markdown.
##
METADATA_SUFFIX = b"\n-->\n"

##
# @brief Number of bytes inspected for the PDF header marker.
##
PDF_HEADER_SCAN_SIZE = 1024

##
# @brief Interactive progress refresh interval in seconds.
##
PROGRESS_TTY_INTERVAL_SECONDS = 1.0

##
# @brief Non-interactive progress refresh interval in seconds.
##
PROGRESS_LOG_INTERVAL_SECONDS = 15.0

# ==============================================================================
# CLASSES
# ==============================================================================


class PdfToMarkdownError(Exception):
  ##
  # @brief Represent an error that prevents trustworthy Markdown publication.
  ##

  pass


class DoclingOutputCapture:
  ##
  # @brief Capture Docling logging and direct stream output for one operation.
  #
  # Normal execution keeps captured output silent until an error occurs. Verbose
  # execution streams logging records while retaining them for diagnostics.
  #
  # @param[in] verbose Whether logging records shall also be displayed live.
  ##
  def __init__(self, verbose):
    self.verbose = verbose
    self.records = []
    self.stdout = io.StringIO()
    self.stderr = io.StringIO()
    self.handler = logging.Handler()
    self.handler.setLevel(logging.DEBUG)
    self.handler.setFormatter(
      logging.Formatter("%(levelname)s %(name)s: %(message)s")
    )
    self.handler.emit = self.emit
    self.logger_states = []
    self.stdout_redirect = None
    self.stderr_redirect = None
    self.live_stderr = None
    self.saved_stdout_fd = None
    self.saved_stderr_fd = None
    self.native_stdout = None
    self.native_stderr = None
    self.native_stdout_text = ""
    self.native_stderr_text = ""

  ##
  # @brief Store one logging record and optionally display it immediately.
  #
  # @param[in] record Logging record emitted by the Python logging framework.
  ##
  def emit(self, record):
    self.records.append(record)
    if self.verbose:
      print(self.handler.format(record), file=self.live_stderr, flush=True)

  ##
  # @brief Capture native writes to stdout and stderr in normal mode.
  #
  # Docling dependencies can write directly to file descriptors 1 and 2,
  # bypassing Python's stream redirection. Temporary files avoid pipe-reader
  # threads and are removed as soon as the capture scope is restored.
  ##
  def capture_native_file_descriptors(self):
    self.saved_stdout_fd = os.dup(1)
    self.saved_stderr_fd = os.dup(2)
    self.native_stdout = tempfile.TemporaryFile(mode="w+b")
    self.native_stderr = tempfile.TemporaryFile(mode="w+b")
    os.dup2(self.native_stdout.fileno(), 1)
    os.dup2(self.native_stderr.fileno(), 2)

  ##
  # @brief Restore native file descriptors and retain captured text.
  ##
  def restore_native_file_descriptors(self):
    restore_errors = []
    saved_descriptors = (
      ("stdout", self.saved_stdout_fd, 1),
      ("stderr", self.saved_stderr_fd, 2),
    )
    for stream_name, saved_fd, target_fd in saved_descriptors:
      if saved_fd is None:
        continue
      try:
        os.dup2(saved_fd, target_fd)
      except OSError as error:
        restore_errors.append((stream_name, error))
      finally:
        os.close(saved_fd)
    self.saved_stdout_fd = None
    self.saved_stderr_fd = None

    for stream_name in ("stdout", "stderr"):
      native_stream = getattr(self, f"native_{stream_name}")
      if native_stream is None:
        continue
      try:
        native_stream.flush()
        native_stream.seek(0)
        text = native_stream.read().decode("utf-8", errors="replace")
        setattr(self, f"native_{stream_name}_text", text)
      except OSError as error:
        restore_errors.append((f"capture {stream_name}", error))
      finally:
        native_stream.close()
        setattr(self, f"native_{stream_name}", None)

    if restore_errors:
      stream_name, error = restore_errors[0]
      raise OSError(
        f"Could not restore native {stream_name} file descriptor"
      ) from error

  ##
  # @brief Restore all logger and stream state changed by this capture.
  ##
  def restore(self):
    if self.stderr_redirect is not None:
      self.stderr_redirect.__exit__(None, None, None)
      self.stdout_redirect.__exit__(None, None, None)
      self.stderr_redirect = None
      self.stdout_redirect = None

    native_restore_error = None
    try:
      if (
        self.saved_stdout_fd is not None
        or self.saved_stderr_fd is not None
        or self.native_stdout is not None
        or self.native_stderr is not None
      ):
        self.restore_native_file_descriptors()
    except BaseException as error:
      native_restore_error = error

    for (
      logger,
      level,
      propagate,
      disabled,
      handler_levels,
    ) in self.logger_states:
      logger.removeHandler(self.handler)
      for handler, handler_level in handler_levels:
        handler.setLevel(handler_level)
      logger.setLevel(level)
      logger.propagate = propagate
      logger.disabled = disabled
    self.logger_states = []
    if native_restore_error is not None:
      raise native_restore_error

  ##
  # @brief Enter the output-capture scope.
  #
  # @return This capture object.
  ##
  def __enter__(self):
    self.live_stderr = sys.stderr
    try:
      for logger_name in DOCLING_LOGGER_NAMES:
        logger = logging.getLogger(logger_name)
        handler_levels = [
          (handler, handler.level) for handler in logger.handlers
        ]
        self.logger_states.append(
          (
            logger,
            logger.level,
            logger.propagate,
            logger.disabled,
            handler_levels,
          )
        )
        for handler, _level in handler_levels:
          handler.setLevel(logging.CRITICAL + 1)
        logger.setLevel(logging.DEBUG)
        logger.propagate = False
        logger.disabled = False
        logger.addHandler(self.handler)

      if not self.verbose:
        self.stdout_redirect = contextlib.redirect_stdout(self.stdout)
        self.stderr_redirect = contextlib.redirect_stderr(self.stderr)
        self.stdout_redirect.__enter__()
        self.stderr_redirect.__enter__()
        self.capture_native_file_descriptors()
    except BaseException:
      self.restore()
      raise
    return self

  ##
  # @brief Leave the output-capture scope and restore logging state.
  #
  # @param[in] _exception_type Exception type, if any.
  # @param[in] _exception_value Exception value, if any.
  # @param[in] _traceback Exception traceback, if any.
  # @return False so exceptions continue to the caller.
  ##
  def __exit__(self, _exception_type, _exception_value, _traceback):
    self.restore()
    return False

  ##
  # @brief Return captured diagnostic text in display order.
  #
  # @return Combined logging, standard-output and standard-error text.
  ##
  def diagnostics(self):
    diagnostics = [self.handler.format(record) for record in self.records]
    for stream in (self.stdout, self.stderr):
      stream_text = stream.getvalue().strip()
      if stream_text:
        diagnostics.append(stream_text)
    for stream_name, stream_text in (
      ("stdout", self.native_stdout_text),
      ("stderr", self.native_stderr_text),
    ):
      stream_text = stream_text.strip()
      if stream_text:
        diagnostics.append(f"Captured native {stream_name}:\n{stream_text}")
    return "\n".join(diagnostics)


class ProgressReporter:
  ##
  # @brief Report segment progress without flooding interactive terminals.
  #
  # @param[in] total_segments Total number of configured segments.
  # @param[in] total_pages Total number of source pages.
  # @param[in] allow_interactive Whether TTY line rewriting is allowed.
  ##
  def __init__(self, total_segments, total_pages, allow_interactive=True):
    self.total_segments = total_segments
    self.total_pages = total_pages
    self.completed_pages = 0
    self.started_at = time.monotonic()
    self.interactive = allow_interactive and sys.stdout.isatty()
    self.line_active = False
    self.current_segment_index = 1
    self.current_start_page = 1
    self.current_end_page = 1
    self.lock = threading.Lock()
    self.stop_event = threading.Event()
    self.finished = False
    self.timer_thread = threading.Thread(
      target=self.run_timer,
      name="pdf-to-markdown-progress",
      daemon=True,
    )
    self.timer_thread.start()

  ##
  # @brief Write one message while the progress lock is held.
  #
  # @param[in] message User-facing progress message.
  ##
  def write_unlocked(self, message):
    if self.interactive:
      sys.stdout.write(f"\r{message}\033[K")
      sys.stdout.flush()
      self.line_active = True
    else:
      print(message, flush=True)

  ##
  # @brief Format elapsed monotonic time as an unbounded clock duration.
  #
  # @param[in] elapsed_seconds Elapsed monotonic seconds.
  # @return Zero-padded hours, minutes and seconds.
  ##
  def format_elapsed(self, elapsed_seconds):
    total_seconds = max(0, int(elapsed_seconds))
    hours, remainder = divmod(total_seconds, 3600)
    minutes, seconds = divmod(remainder, 60)
    return f"{hours:02d}:{minutes:02d}:{seconds:02d}"

  ##
  # @brief Build one stable progress message for every output mode.
  #
  # @param[in] segment_index One-based segment index.
  # @param[in] start_page Inclusive segment start.
  # @param[in] end_page Inclusive segment end.
  # @return User-facing progress message.
  ##
  def progress_message(self, segment_index, start_page, end_page):
    percentage = 100 * self.completed_pages / self.total_pages
    elapsed = self.format_elapsed(time.monotonic() - self.started_at)
    return (
      f"PDF to Markdown | {percentage:.1f}% "
      f"({self.completed_pages}/{self.total_pages} pages) | "
      f"segment {segment_index}/{self.total_segments} "
      f"[pages {start_page}-{end_page}] | elapsed {elapsed}"
    )

  ##
  # @brief Build the current progress message while the lock is held.
  #
  # @return User-facing progress message for the active segment.
  ##
  def current_progress_message_unlocked(self):
    return self.progress_message(
      self.current_segment_index,
      self.current_start_page,
      self.current_end_page,
    )

  ##
  # @brief Refresh progress from the timer thread.
  ##
  def refresh_from_timer(self):
    with self.lock:
      if self.finished:
        return
      self.write_unlocked(self.current_progress_message_unlocked())

  ##
  # @brief Run periodic total-duration progress refreshes.
  ##
  def run_timer(self):
    interval = (
      PROGRESS_TTY_INTERVAL_SECONDS
      if self.interactive
      else PROGRESS_LOG_INTERVAL_SECONDS
    )
    while not self.stop_event.wait(interval):
      self.refresh_from_timer()

  ##
  # @brief Report the segment about to be processed.
  #
  # @param[in] segment_index One-based segment index.
  # @param[in] start_page Inclusive segment start.
  # @param[in] end_page Inclusive segment end.
  ##
  def segment_started(self, segment_index, start_page, end_page):
    with self.lock:
      if self.finished:
        return
      self.current_segment_index = segment_index
      self.current_start_page = start_page
      self.current_end_page = end_page
      self.write_unlocked(self.current_progress_message_unlocked())

  ##
  # @brief Report one completed segment.
  #
  # @param[in] segment_index One-based segment index.
  # @param[in] start_page Inclusive segment start.
  # @param[in] end_page Inclusive segment end.
  ##
  def segment_completed(self, segment_index, start_page, end_page):
    with self.lock:
      if self.finished:
        return
      self.completed_pages += end_page - start_page + 1
      self.current_segment_index = segment_index
      self.current_start_page = start_page
      self.current_end_page = end_page
      self.write_unlocked(self.current_progress_message_unlocked())

  ##
  # @brief Finish an interactive progress line before an error or return.
  ##
  def finish(self):
    with self.lock:
      if self.finished:
        return
      self.finished = True
    self.stop_event.set()
    if threading.current_thread() is not self.timer_thread:
      self.timer_thread.join()
    with self.lock:
      if self.interactive and self.line_active:
        sys.stdout.write("\n")
        sys.stdout.flush()
        self.line_active = False


# ==============================================================================
# FUNCTIONS
# ==============================================================================


##
# @brief Parse command-line arguments.
#
# @param[in] arguments Optional argument list excluding the executable name.
# @return Parsed command-line arguments.
##
def parse_args(arguments=None):
  parser = argparse.ArgumentParser(
    description="Convert one local PDF file to one Markdown file with Docling."
  )
  parser.add_argument(
    "--input",
    required=True,
    type=Path,
    help="Local input PDF path.",
  )
  parser.add_argument(
    "--output",
    required=True,
    type=Path,
    help="Destination Markdown path.",
  )
  parser.add_argument(
    "--force",
    action="store_true",
    help="Atomically replace a non-matching existing output.",
  )
  parser.add_argument(
    "--verbose",
    action="store_true",
    help="Show Docling logs during conversion.",
  )
  parser.add_argument(
    "--pages-per-segment",
    default=10,
    type=positive_integer,
    help="Base number of pages per segment (default: 10).",
  )
  parser.add_argument(
    "--page-ranges",
    default=None,
    help=("Forced inclusive page ranges, for example '50-53,600-601'."),
  )
  return parser.parse_args(arguments)


##
# @brief Parse arguments for one internal segment worker.
#
# @param[in] arguments Argument list including the hidden worker marker.
# @return Parsed worker arguments.
##
def parse_worker_args(arguments):
  parser = argparse.ArgumentParser(
    description="Convert one PDF page segment with Docling."
  )
  parser.add_argument(
    "--segment-worker",
    action="store_true",
    required=True,
    help=argparse.SUPPRESS,
  )
  parser.add_argument(
    "--input",
    required=True,
    type=Path,
    help=argparse.SUPPRESS,
  )
  parser.add_argument(
    "--output",
    required=True,
    type=Path,
    help=argparse.SUPPRESS,
  )
  parser.add_argument(
    "--start-page",
    required=True,
    type=positive_integer,
    help=argparse.SUPPRESS,
  )
  parser.add_argument(
    "--end-page",
    required=True,
    type=positive_integer,
    help=argparse.SUPPRESS,
  )
  parser.add_argument(
    "--verbose",
    action="store_true",
    help=argparse.SUPPRESS,
  )
  return parser.parse_args(arguments)


##
# @brief Parse a strictly positive integer command-line value.
#
# @param[in] value Text supplied by the command-line parser.
# @return Parsed positive integer.
##
def positive_integer(value):
  try:
    parsed_value = int(value)
  except (TypeError, ValueError) as error:
    raise argparse.ArgumentTypeError(
      "value must be a positive integer"
    ) from error
  if parsed_value <= 0:
    raise argparse.ArgumentTypeError("value must be a positive integer")
  return parsed_value


##
# @brief Serialize an object as deterministic compact JSON.
#
# @param[in] value JSON-compatible value.
# @return Canonical UTF-8 JSON bytes.
##
def canonical_json(value):
  return json.dumps(
    value,
    ensure_ascii=True,
    separators=(",", ":"),
    sort_keys=True,
  ).encode("utf-8")


##
# @brief Calculate the SHA-256 digest of a file.
#
# @param[in] file_path Path to the file to hash.
# @return Lowercase hexadecimal SHA-256 digest.
##
def sha256_file(file_path):
  digest = hashlib.sha256()
  with file_path.open("rb") as file_handle:
    for chunk in iter(lambda: file_handle.read(1024 * 1024), b""):
      digest.update(chunk)
  return digest.hexdigest()


##
# @brief Calculate the deterministic conversion cache key.
#
# @param[in] input_sha256 SHA-256 digest of the input PDF.
# @param[in] conversion_options Canonical conversion options.
# @return Lowercase hexadecimal cache-key digest.
##
def build_cache_key(input_sha256, conversion_options):
  key_material = {
    "docling_version": get_docling_version(),
    "input_sha256": input_sha256,
    "options": conversion_options,
    "tool_version": TOOL_VERSION,
  }
  return hashlib.sha256(canonical_json(key_material)).hexdigest()


##
# @brief Build provenance metadata for one generated Markdown body.
#
# @param[in] input_sha256 SHA-256 digest of the input PDF.
# @param[in] markdown_sha256 SHA-256 digest of the Markdown body.
# @param[in] conversion_options Canonical conversion options.
# @return Provenance metadata dictionary.
##
def build_metadata(input_sha256, markdown_sha256, conversion_options):
  return {
    "cache_key": build_cache_key(input_sha256, conversion_options),
    "docling_version": get_docling_version(),
    "input_sha256": input_sha256,
    "markdown_sha256": markdown_sha256,
    "options": conversion_options,
    "schema_version": 1,
    "tool_version": TOOL_VERSION,
  }


##
# @brief Read provenance metadata and hash the Markdown body as a stream.
#
# @param[in] output_path Versioned Markdown output path.
# @return Tuple containing metadata and body digest, or (None, None) if
# malformed.
##
def read_metadata_and_body_sha256(output_path):
  with output_path.open("rb") as output_handle:
    if output_handle.read(len(METADATA_PREFIX)) != METADATA_PREFIX:
      return None, None

    delimiter_tail_length = len(METADATA_SUFFIX) - 1
    pending = b""
    with tempfile.TemporaryFile(mode="w+b") as metadata_handle:
      while True:
        chunk = output_handle.read(64 * 1024)
        if not chunk:
          return None, None
        data = pending + chunk
        metadata_end = data.find(METADATA_SUFFIX)
        if metadata_end >= 0:
          metadata_handle.write(data[:metadata_end])
          body_hash = hashlib.sha256()
          body_hash.update(data[metadata_end + len(METADATA_SUFFIX) :])
          for body_chunk in iter(
            lambda: output_handle.read(1024 * 1024),
            b"",
          ):
            body_hash.update(body_chunk)

          metadata_handle.seek(0)
          try:
            metadata = json.load(metadata_handle)
          except (UnicodeDecodeError, json.JSONDecodeError):
            return None, None
          if not isinstance(metadata, dict):
            return None, None
          return metadata, body_hash.hexdigest()

        if len(data) > delimiter_tail_length:
          metadata_handle.write(data[:-delimiter_tail_length])
          pending = data[-delimiter_tail_length:]
        else:
          pending = data


##
# @brief Check whether an existing output exactly matches a conversion key.
#
# @param[in] output_path Existing Markdown output path.
# @param[in] input_sha256 SHA-256 digest of the current input PDF.
# @return True when metadata and Markdown body integrity match.
##
def is_cache_hit(output_path, input_sha256, conversion_options):
  metadata, body_sha256 = read_metadata_and_body_sha256(output_path)
  if metadata is None:
    return False

  return metadata == build_metadata(
    input_sha256,
    body_sha256,
    conversion_options,
  )


##
# @brief Stream exporter text as deterministic UTF-8 Markdown.
#
# Newline translation and removal of trailing newlines are performed without
# loading the complete segment Markdown into the parent process.
#
# @param[in] markdown_path Segment Markdown path.
# @param[out] output_handle Open binary concatenation handle.
##
def stream_normalized_markdown(markdown_path, output_handle):
  pending_newlines = 0
  with markdown_path.open(
    "r",
    encoding="utf-8",
    newline=None,
  ) as markdown_handle:
    while True:
      chunk = markdown_handle.read(1024 * 1024)
      if not chunk:
        break
      chunk = ("\n" * pending_newlines) + chunk
      content = chunk.rstrip("\n")
      pending_newlines = len(chunk) - len(content)
      if content:
        output_handle.write(content.encode("utf-8"))

  output_handle.write(b"\n")


##
# @brief Parse forced inclusive, one-based page ranges.
#
# @param[in] ranges_text Comma-separated range text.
# @return Ordered list of inclusive page-range tuples.
##
def parse_page_ranges(ranges_text):
  if ranges_text is None:
    return []
  if not ranges_text.strip():
    raise PdfToMarkdownError("Page ranges must not be empty.")

  parsed_ranges = []
  for range_text in ranges_text.split(","):
    match = re.fullmatch(r"\s*(\d+)\s*-\s*(\d+)\s*", range_text)
    if match is None:
      raise PdfToMarkdownError(
        f"Invalid page range syntax: {range_text.strip() or '<empty>'}"
      )
    start_page = int(match.group(1))
    end_page = int(match.group(2))
    if start_page < 1 or end_page < 1 or start_page > end_page:
      raise PdfToMarkdownError(f"Invalid page range: {range_text.strip()}")

    if parsed_ranges:
      previous_start, previous_end = parsed_ranges[-1]
      if (start_page, end_page) == (previous_start, previous_end):
        raise PdfToMarkdownError(f"Duplicate page range: {range_text.strip()}")
      if start_page <= previous_start:
        raise PdfToMarkdownError(
          "Forced page ranges must be ordered by start page."
        )
      if start_page <= previous_end:
        raise PdfToMarkdownError(
          f"Overlapping page range: {range_text.strip()}"
        )

    parsed_ranges.append((start_page, end_page))

  return parsed_ranges


##
# @brief Append base-sized segments for one unforced page interval.
#
# @param[in,out] segments Destination segment list.
# @param[in] start_page Inclusive interval start.
# @param[in] end_page Inclusive interval end.
# @param[in] pages_per_segment Base segment size.
##
def append_base_segments(segments, start_page, end_page, pages_per_segment):
  next_page = start_page
  while next_page <= end_page:
    segment_end = min(
      next_page + pages_per_segment - 1,
      end_page,
    )
    segments.append((next_page, segment_end))
    next_page = segment_end + 1


##
# @brief Build the complete, non-overlapping segment partition.
#
# @param[in] total_pages Number of pages in the source PDF.
# @param[in] pages_per_segment Base segment size.
# @param[in] forced_ranges Ordered forced ranges.
# @return Complete ordered segment list covering every source page once.
##
def build_segments(total_pages, pages_per_segment, forced_ranges):
  if total_pages < 1:
    raise PdfToMarkdownError("Input PDF must contain at least one page.")
  if pages_per_segment < 1:
    raise PdfToMarkdownError("Pages per segment must be a positive integer.")

  previous_range = None
  for start_page, end_page in forced_ranges:
    if start_page < 1 or end_page < start_page:
      raise PdfToMarkdownError(f"Invalid page range: {start_page}-{end_page}")
    if end_page > total_pages:
      raise PdfToMarkdownError(
        f"Page range {start_page}-{end_page} exceeds the "
        f"{total_pages}-page document."
      )
    if previous_range is not None:
      previous_start, previous_end = previous_range
      if (start_page, end_page) == previous_range:
        raise PdfToMarkdownError(
          f"Duplicate page range: {start_page}-{end_page}"
        )
      if start_page <= previous_start:
        raise PdfToMarkdownError(
          "Forced page ranges must be ordered by start page."
        )
      if start_page <= previous_end:
        raise PdfToMarkdownError(
          f"Overlapping page range: {start_page}-{end_page}"
        )
    previous_range = (start_page, end_page)

  segments = []
  next_page = 1
  for start_page, end_page in forced_ranges:
    append_base_segments(
      segments,
      next_page,
      start_page - 1,
      pages_per_segment,
    )
    segments.append((start_page, end_page))
    next_page = end_page + 1

  append_base_segments(
    segments,
    next_page,
    total_pages,
    pages_per_segment,
  )

  expected_page = 1
  for start_page, end_page in segments:
    if start_page != expected_page:
      raise PdfToMarkdownError(
        "Generated page segments do not provide complete coverage."
      )
    expected_page = end_page + 1
  if expected_page != total_pages + 1:
    raise PdfToMarkdownError(
      "Generated page segments do not cover the complete PDF."
    )

  return segments


##
# @brief Count pages in a local PDF through the installed PDFium binding.
#
# @param[in] input_path Resolved PDF path.
# @return Number of pages in the PDF.
##
def count_pdf_pages(input_path):
  try:
    pdfium = importlib.import_module("pypdfium2")
    document = pdfium.PdfDocument(input_path)
    try:
      return len(document)
    finally:
      document.close()
  except Exception as error:
    raise PdfToMarkdownError(
      f"Could not count pages in input PDF: {input_path}"
    ) from error


##
# @brief Build conversion options including the final segment partition.
#
# @param[in] pages_per_segment Base segment size.
# @param[in] forced_ranges Ordered forced ranges.
# @param[in] segments Complete segment partition.
# @return Canonical conversion options for provenance and caching.
##
def build_conversion_options(pages_per_segment, forced_ranges, segments):
  return {
    **CONVERSION_OPTIONS,
    "segmentation_version": SEGMENTATION_VERSION,
    "pages_per_segment": pages_per_segment,
    "forced_page_ranges": [list(item) for item in forced_ranges],
    "segments": [list(item) for item in segments],
    "segment_separator": "lf",
  }


##
# @brief Resolve and validate input and output paths.
#
# @param[in] input_path User-provided input PDF path.
# @param[in] output_path User-provided output Markdown path.
# @return Tuple of resolved input and output paths.
##
def validate_paths(input_path, output_path):
  expanded_input = input_path.expanduser()
  expanded_output = output_path.expanduser()

  try:
    resolved_input = expanded_input.resolve(strict=True)
  except FileNotFoundError as error:
    raise PdfToMarkdownError(
      f"Input PDF does not exist: {expanded_input}"
    ) from error

  resolved_output = (
    expanded_output.parent.resolve(strict=False) / expanded_output.name
  )

  if not resolved_input.is_file():
    raise PdfToMarkdownError(
      f"Input PDF is not a regular file: {resolved_input}"
    )
  if resolved_input.suffix.lower() != ".pdf":
    raise PdfToMarkdownError(f"Input path must end in .pdf: {resolved_input}")
  if resolved_output.suffix.lower() != ".md":
    raise PdfToMarkdownError(f"Output path must end in .md: {resolved_output}")
  if resolved_output.is_symlink():
    raise PdfToMarkdownError(
      f"Output path must not be a symbolic link: {resolved_output}"
    )
  if resolved_output.exists() and not resolved_output.is_file():
    raise PdfToMarkdownError(
      f"Output path exists and is not a regular file: {resolved_output}"
    )
  if resolved_output.exists() and os.path.samefile(
    resolved_input, resolved_output
  ):
    raise PdfToMarkdownError(
      "Input and output must not identify the same file."
    )

  with resolved_input.open("rb") as input_handle:
    header = input_handle.read(PDF_HEADER_SCAN_SIZE)
  if b"%PDF-" not in header:
    raise PdfToMarkdownError(
      f"Input file does not contain a PDF header: {resolved_input}"
    )

  return resolved_input, resolved_output


##
# @brief Return the installed Docling package version.
#
# The result is cached because the version participates in every cache and
# provenance check performed during one process execution.
#
# @return Installed Docling package version.
##
@functools.cache
def get_docling_version():
  try:
    return importlib.metadata.version("docling")
  except importlib.metadata.PackageNotFoundError as error:
    raise PdfToMarkdownError(
      "Docling is required but is not installed."
    ) from error


##
# @brief Return the user-writable RapidOCR model cache directory.
#
# This follows Docling's own default cache root instead of RapidOCR's default
# package directory, which is read-only in the devcontainer.
#
# @return RapidOCR model cache path below the current user's home directory.
##
def rapidocr_model_cache_path():
  return Path.home() / ".cache" / "docling" / "rapidocr"


##
# @brief Create the configured Docling converter.
#
# Pictures are rendered and embedded as data URIs so publication remains one
# atomic Markdown file and no external asset path can collide.
#
# @return Tuple containing the converter and imported Docling modules.
##
def create_docling_converter():
  get_docling_version()

  base_models = importlib.import_module("docling.datamodel.base_models")
  pipeline_module = importlib.import_module(
    "docling.datamodel.pipeline_options"
  )
  converter_module = importlib.import_module("docling.document_converter")
  document_module = importlib.import_module("docling_core.types.doc")

  pipeline_options = pipeline_module.PdfPipelineOptions()
  pipeline_options.generate_page_images = CONVERSION_OPTIONS[
    "generate_page_images"
  ]
  pipeline_options.generate_picture_images = CONVERSION_OPTIONS[
    "generate_picture_images"
  ]
  pipeline_options.images_scale = CONVERSION_OPTIONS["images_scale"]
  pipeline_options.ocr_batch_size = CONVERSION_OPTIONS["ocr_batch_size"]
  pipeline_options.layout_batch_size = CONVERSION_OPTIONS["layout_batch_size"]
  pipeline_options.table_batch_size = CONVERSION_OPTIONS["table_batch_size"]
  pipeline_options.queue_max_size = CONVERSION_OPTIONS["queue_max_size"]
  pipeline_options.ocr_options = pipeline_module.RapidOcrOptions(
    backend=CONVERSION_OPTIONS["ocr_backend"],
    lang=[CONVERSION_OPTIONS["ocr_language"]],
    rapidocr_params={
      "Global.model_root_dir": str(rapidocr_model_cache_path()),
    },
  )

  converter = converter_module.DocumentConverter(
    allowed_formats=[base_models.InputFormat.PDF],
    format_options={
      base_models.InputFormat.PDF: converter_module.PdfFormatOption(
        pipeline_options=pipeline_options
      )
    },
  )
  return converter, base_models, document_module


##
# @brief Convert one segment inside a fresh worker process.
#
# @param[in] input_path Resolved input PDF path.
# @param[out] segment_path Temporary segment Markdown path.
# @param[in] start_page Inclusive segment start.
# @param[in] end_page Inclusive segment end.
# @param[in] verbose Whether Docling output shall be displayed.
##
def convert_segment_with_docling(
  input_path,
  segment_path,
  start_page,
  end_page,
  verbose=False,
):
  output_capture = None
  try:
    with DoclingOutputCapture(verbose) as output_capture:
      converter, base_models, document_module = create_docling_converter()
      result = converter.convert(
        input_path,
        raises_on_error=True,
        page_range=(start_page, end_page),
      )
      if result.status != base_models.ConversionStatus.SUCCESS:
        raise PdfToMarkdownError(
          f"Docling returned non-success status: {result.status}"
        )
      result.document.save_as_markdown(
        segment_path,
        image_mode=document_module.ImageRefMode.EMBEDDED,
      )
  except Exception as error:
    diagnostics = (
      output_capture.diagnostics() if output_capture is not None else ""
    )
    if diagnostics:
      raise PdfToMarkdownError(
        f"Docling segment conversion failed: {error}\n"
        f"Docling diagnostics:\n{diagnostics}"
      ) from error
    raise PdfToMarkdownError(
      f"Docling segment conversion failed: {error}"
    ) from error


##
# @brief Execute one internal segment worker.
#
# @param[in] arguments Argument list including the hidden worker marker.
# @return Process exit code.
##
def segment_worker_main(arguments):
  parsed_arguments = parse_worker_args(arguments)
  try:
    input_path, output_path = validate_paths(
      parsed_arguments.input,
      parsed_arguments.output,
    )
    if parsed_arguments.end_page < parsed_arguments.start_page:
      raise PdfToMarkdownError(
        "Segment end page must not precede its start page."
      )
    convert_segment_with_docling(
      input_path,
      output_path,
      parsed_arguments.start_page,
      parsed_arguments.end_page,
      verbose=parsed_arguments.verbose,
    )
    if not output_path.is_file():
      raise PdfToMarkdownError(
        f"Docling did not create segment output: {output_path}"
      )
  except Exception as error:
    print(
      f"❌ PDF to Markdown segment worker failed: {error}",
      file=sys.stderr,
      flush=True,
    )
    return 1
  return 0


##
# @brief Terminate and reap an active segment worker.
#
# @param[in] process Running worker process.
##
def terminate_segment_worker(process):
  if process.poll() is not None:
    process.communicate()
    return

  process.terminate()
  try:
    process.communicate(timeout=5)
  except subprocess.TimeoutExpired:
    process.kill()
    process.communicate()


##
# @brief Run one segment worker and validate its Markdown output.
#
# @param[in] input_path Resolved input PDF path.
# @param[out] segment_path Temporary segment Markdown path.
# @param[in] start_page Inclusive segment start.
# @param[in] end_page Inclusive segment end.
# @param[in] verbose Whether worker output shall be forwarded after completion.
# @return None on successful worker completion.
##
def run_segment_worker(
  input_path,
  segment_path,
  start_page,
  end_page,
  verbose=False,
):
  command = [
    sys.executable,
    str(Path(__file__).resolve()),
    "--segment-worker",
    "--input",
    str(input_path),
    "--output",
    str(segment_path),
    "--start-page",
    str(start_page),
    "--end-page",
    str(end_page),
  ]
  if verbose:
    command.append("--verbose")

  process = subprocess.Popen(
    command,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    text=True,
    encoding="utf-8",
    errors="replace",
  )
  try:
    stdout, stderr = process.communicate()
  except BaseException:
    terminate_segment_worker(process)
    raise

  if verbose:
    if stdout:
      print(stdout, end="", flush=True)
    if stderr:
      print(stderr, end="", file=sys.stderr, flush=True)

  if process.returncode != 0:
    diagnostics = "\n".join(
      stream.strip() for stream in (stdout, stderr) if stream.strip()
    )
    if process.returncode < 0:
      signal_name = signal.Signals(-process.returncode).name
      termination = f"terminated by signal {signal_name}"
    elif process.returncode == 128 + signal.SIGKILL:
      termination = "exited with status 137 (likely SIGKILL)"
    else:
      termination = f"exited with status {process.returncode}"
    message = f"Segment worker {termination}."
    if diagnostics:
      message += f"\nWorker diagnostics:\n{diagnostics}"
    raise PdfToMarkdownError(message)
  if not segment_path.is_file():
    raise PdfToMarkdownError(
      f"Segment worker completed without creating output: {segment_path}"
    )


##
# @brief Convert all configured page segments and concatenate their Markdown.
#
# @param[in] input_path Resolved input PDF path.
# @param[out] body_path Temporary concatenated Markdown body path.
# @param[in] segments Complete ordered segment partition.
# @return SHA-256 digest of the concatenated Markdown body.
##
def convert_with_docling(input_path, body_path, segments, verbose=False):
  total_pages = sum(
    end_page - start_page + 1 for start_page, end_page in segments
  )
  progress = ProgressReporter(
    len(segments),
    total_pages,
    allow_interactive=not verbose,
  )
  try:
    print(
      "PDF to Markdown: converting segments in isolated workers...",
      flush=True,
    )

    with body_path.open("wb") as body_handle:
      for segment_index, (start_page, end_page) in enumerate(
        segments,
        start=1,
      ):
        segment_path = body_path.parent / (
          f"segment-{segment_index:06d}-{start_page}-{end_page}.md"
        )
        progress.segment_started(segment_index, start_page, end_page)

        try:
          run_segment_worker(
            input_path,
            segment_path,
            start_page,
            end_page,
            verbose=verbose,
          )
          stream_normalized_markdown(segment_path, body_handle)
          body_handle.flush()
        except Exception as error:
          message = (
            f"Segment {segment_index}/{len(segments)} "
            f"(pages {start_page}-{end_page}) failed: {error}"
          )
          raise PdfToMarkdownError(message) from error
        finally:
          segment_path.unlink(missing_ok=True)

        progress.segment_completed(segment_index, start_page, end_page)

    return sha256_file(body_path)
  finally:
    progress.finish()


##
# @brief Create an empty staging file beside the final destination.
#
# @param[in] output_path Final Markdown output path.
# @return Staging file path on the destination filesystem.
##
def create_staging_file(output_path):
  file_descriptor, staging_name = tempfile.mkstemp(
    dir=output_path.parent,
    prefix=f".{output_path.name}.",
    suffix=".tmp",
  )
  os.close(file_descriptor)
  return Path(staging_name)


##
# @brief Create a private temporary directory beside the final output.
#
# @param[in] output_path Final Markdown output path.
# @return Temporary directory path.
##
def create_temporary_directory(output_path):
  return Path(
    tempfile.mkdtemp(
      dir=output_path.parent,
      prefix=f".{output_path.name}.segments-",
    )
  )


##
# @brief Publish metadata and a streamed Markdown body to the staging file.
#
# @param[in] staging_path Final staging file path.
# @param[in] input_sha256 Input PDF digest.
# @param[in] body_path Concatenated Markdown body path.
# @param[in] body_sha256 Markdown body digest.
# @param[in] conversion_options Canonical conversion options.
##
def write_final_staging_file(
  staging_path,
  input_sha256,
  body_path,
  body_sha256,
  conversion_options,
):
  metadata = build_metadata(
    input_sha256,
    body_sha256,
    conversion_options,
  )
  metadata_data = METADATA_PREFIX + canonical_json(metadata) + METADATA_SUFFIX

  with staging_path.open("wb") as output_handle:
    output_handle.write(metadata_data)
    with body_path.open("rb") as body_handle:
      shutil.copyfileobj(body_handle, output_handle)


##
# @brief Flush staged file content and metadata to stable storage.
#
# @param[in] staging_path Staged Markdown path.
##
def sync_staging_file(staging_path):
  os.chmod(staging_path, 0o644)
  with staging_path.open("rb") as staging_handle:
    os.fsync(staging_handle.fileno())


##
# @brief Publish a complete staged file without an implicit overwrite.
#
# @param[in] staging_path Complete staged Markdown path.
# @param[in] output_path Final Markdown output path.
# @param[in] force Whether an atomic replacement was explicitly requested.
##
def publish_staging_file(staging_path, output_path, force):
  if force:
    os.replace(staging_path, output_path)
    return

  try:
    os.link(staging_path, output_path)
  except FileExistsError as error:
    raise PdfToMarkdownError(
      f"Output appeared during conversion; refusing to overwrite: {output_path}"
    ) from error
  staging_path.unlink()


##
# @brief Execute cached, staged PDF-to-Markdown conversion.
#
# @param[in] input_path User-provided input PDF path.
# @param[in] output_path User-provided output Markdown path.
# @param[in] force Whether to replace a non-matching existing output.
# @return "cached" when reused or "converted" when newly published.
##
def convert_pdf_to_markdown(
  input_path,
  output_path,
  force=False,
  pages_per_segment=10,
  forced_ranges=None,
  segments=None,
  conversion_options=None,
  verbose=False,
):
  resolved_input, resolved_output = validate_paths(input_path, output_path)
  input_sha256 = sha256_file(resolved_input)
  if forced_ranges is None:
    forced_ranges = []
  if segments is None:
    total_pages = count_pdf_pages(resolved_input)
    segments = build_segments(
      total_pages,
      pages_per_segment,
      forced_ranges,
    )
  if conversion_options is None:
    conversion_options = build_conversion_options(
      pages_per_segment,
      forced_ranges,
      segments,
    )

  if resolved_output.exists():
    if is_cache_hit(
      resolved_output,
      input_sha256,
      conversion_options,
    ):
      if sha256_file(resolved_input) != input_sha256:
        raise PdfToMarkdownError("Input PDF changed while checking the cache.")
      return "cached"
    if not force:
      raise PdfToMarkdownError(
        "Output exists but does not match this conversion. "
        f"Use --force to replace it: {resolved_output}"
      )

  resolved_output.parent.mkdir(parents=True, exist_ok=True)
  staging_path = create_staging_file(resolved_output)
  temporary_directory = None

  try:
    temporary_directory = create_temporary_directory(resolved_output)
    body_path = temporary_directory / "markdown-body.md"
    body_sha256 = convert_with_docling(
      resolved_input,
      body_path,
      segments,
      verbose=verbose,
    )
    if sha256_file(resolved_input) != input_sha256:
      raise PdfToMarkdownError("Input PDF changed during conversion.")

    write_final_staging_file(
      staging_path,
      input_sha256,
      body_path,
      body_sha256,
      conversion_options,
    )
    sync_staging_file(staging_path)
    publish_staging_file(staging_path, resolved_output, force)
  finally:
    staging_path.unlink(missing_ok=True)
    if temporary_directory is not None:
      shutil.rmtree(temporary_directory, ignore_errors=True)

  return "converted"


##
# @brief Request cooperative termination after a SIGTERM.
#
# Raising the project error lets active conversion scopes execute their
# cleanup handlers while leaving worker processes on their normal signal
# behavior.
#
# @param[in] _signal_number Received signal number.
# @param[in] _frame Interrupted Python stack frame.
##
def handle_sigterm(_signal_number, _frame):
  raise PdfToMarkdownError("PDF to Markdown interrupted by SIGTERM.")


##
# @brief Run the command-line conversion workflow.
#
# @param[in] arguments Optional argument list excluding the executable name.
# @return Process exit code.
##
def main(arguments=None):
  if arguments is None:
    arguments = sys.argv[1:]
  if "--segment-worker" in arguments:
    return segment_worker_main(arguments)

  previous_sigterm_handler = signal.getsignal(signal.SIGTERM)
  signal.signal(signal.SIGTERM, handle_sigterm)
  try:
    parsed_arguments = parse_args(arguments)
    forced_ranges = parse_page_ranges(parsed_arguments.page_ranges)
    input_path, _output_path = validate_paths(
      parsed_arguments.input,
      parsed_arguments.output,
    )
    total_pages = count_pdf_pages(input_path)
    segments = build_segments(
      total_pages,
      parsed_arguments.pages_per_segment,
      forced_ranges,
    )
    conversion_options = build_conversion_options(
      parsed_arguments.pages_per_segment,
      forced_ranges,
      segments,
    )
    outcome = convert_pdf_to_markdown(
      parsed_arguments.input,
      parsed_arguments.output,
      force=parsed_arguments.force,
      pages_per_segment=parsed_arguments.pages_per_segment,
      forced_ranges=forced_ranges,
      segments=segments,
      conversion_options=conversion_options,
      verbose=parsed_arguments.verbose,
    )
  except Exception as error:
    print(f"❌ PDF to Markdown failed: {error}", file=sys.stderr, flush=True)
    return 1
  finally:
    signal.signal(signal.SIGTERM, previous_sigterm_handler)

  if outcome == "cached":
    print(
      f"✅ PDF to Markdown cache hit: {parsed_arguments.output}", flush=True
    )
  else:
    print(f"✅ Markdown written: {parsed_arguments.output}", flush=True)
  return 0


# ==============================================================================
# SCRIPT ENTRY POINT
# ==============================================================================

if __name__ == "__main__":
  sys.exit(main())
