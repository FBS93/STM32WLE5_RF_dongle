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
import functools
import hashlib
import importlib
import importlib.metadata
import json
import os
import sys
import tempfile
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
TOOL_VERSION = "1.0.0"

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

# ==============================================================================
# CLASSES
# ==============================================================================


class PdfToMarkdownError(Exception):
  ##
  # @brief Represent an error that prevents trustworthy Markdown publication.
  ##

  pass


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
  return parser.parse_args(arguments)


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
# @return Lowercase hexadecimal cache-key digest.
##
def build_cache_key(input_sha256):
  key_material = {
    "docling_version": get_docling_version(),
    "input_sha256": input_sha256,
    "options": CONVERSION_OPTIONS,
    "tool_version": TOOL_VERSION,
  }
  return hashlib.sha256(canonical_json(key_material)).hexdigest()


##
# @brief Build provenance metadata for one generated Markdown body.
#
# @param[in] input_sha256 SHA-256 digest of the input PDF.
# @param[in] markdown_body Generated Markdown body bytes.
# @return Provenance metadata dictionary.
##
def build_metadata(input_sha256, markdown_body):
  return {
    "cache_key": build_cache_key(input_sha256),
    "docling_version": get_docling_version(),
    "input_sha256": input_sha256,
    "markdown_sha256": hashlib.sha256(markdown_body).hexdigest(),
    "options": CONVERSION_OPTIONS,
    "schema_version": 1,
    "tool_version": TOOL_VERSION,
  }


##
# @brief Add deterministic provenance metadata to generated Markdown.
#
# @param[in] input_sha256 SHA-256 digest of the input PDF.
# @param[in] markdown_body Generated Markdown body bytes.
# @return Complete versioned Markdown bytes.
##
def add_metadata(input_sha256, markdown_body):
  metadata = build_metadata(input_sha256, markdown_body)
  return (
    METADATA_PREFIX + canonical_json(metadata) + METADATA_SUFFIX + markdown_body
  )


##
# @brief Split versioned Markdown into metadata and body.
#
# @param[in] output_data Complete output bytes.
# @return Tuple containing metadata and body, or (None, None) if malformed.
##
def split_versioned_markdown(output_data):
  if not output_data.startswith(METADATA_PREFIX):
    return None, None

  metadata_end = output_data.find(METADATA_SUFFIX, len(METADATA_PREFIX))
  if metadata_end < 0:
    return None, None

  metadata_data = output_data[len(METADATA_PREFIX) : metadata_end]
  markdown_body = output_data[metadata_end + len(METADATA_SUFFIX) :]

  try:
    metadata = json.loads(metadata_data.decode("utf-8"))
  except (UnicodeDecodeError, json.JSONDecodeError):
    return None, None

  if not isinstance(metadata, dict):
    return None, None
  return metadata, markdown_body


##
# @brief Check whether an existing output exactly matches a conversion key.
#
# @param[in] output_path Existing Markdown output path.
# @param[in] input_sha256 SHA-256 digest of the current input PDF.
# @return True when metadata and Markdown body integrity match.
##
def is_cache_hit(output_path, input_sha256):
  metadata, markdown_body = split_versioned_markdown(output_path.read_bytes())
  if metadata is None:
    return False

  return metadata == build_metadata(input_sha256, markdown_body)


##
# @brief Normalize exporter text to deterministic UTF-8 Markdown bytes.
#
# @param[in] markdown_text Markdown text emitted by Docling.
# @return UTF-8 Markdown bytes with LF endings and one final newline.
##
def normalize_markdown(markdown_text):
  normalized = markdown_text.replace("\r\n", "\n").replace("\r", "\n")
  normalized = normalized.rstrip("\n") + "\n"
  return normalized.encode("utf-8")


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
# @brief Convert a PDF into staged Markdown through the installed Docling API.
#
# Pictures are rendered and embedded as data URIs so publication remains one
# atomic Markdown file and no external asset path can collide.
#
# @param[in] input_path Resolved input PDF path.
# @param[out] staging_path Staged Markdown path.
##
def convert_with_docling(input_path, staging_path):
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
  result = converter.convert(input_path, raises_on_error=True)

  if result.status != base_models.ConversionStatus.SUCCESS:
    raise PdfToMarkdownError(
      f"Docling returned non-success status: {result.status}"
    )

  result.document.save_as_markdown(
    staging_path,
    image_mode=document_module.ImageRefMode.EMBEDDED,
  )


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
def convert_pdf_to_markdown(input_path, output_path, force=False):
  resolved_input, resolved_output = validate_paths(input_path, output_path)
  input_sha256 = sha256_file(resolved_input)

  if resolved_output.exists():
    if is_cache_hit(resolved_output, input_sha256):
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

  try:
    convert_with_docling(resolved_input, staging_path)
    if sha256_file(resolved_input) != input_sha256:
      raise PdfToMarkdownError("Input PDF changed during conversion.")

    markdown_text = staging_path.read_text(encoding="utf-8")
    markdown_body = normalize_markdown(markdown_text)
    output_data = add_metadata(input_sha256, markdown_body)
    staging_path.write_bytes(output_data)
    sync_staging_file(staging_path)
    publish_staging_file(staging_path, resolved_output, force)
  finally:
    staging_path.unlink(missing_ok=True)

  return "converted"


##
# @brief Run the command-line conversion workflow.
#
# @param[in] arguments Optional argument list excluding the executable name.
# @return Process exit code.
##
def main(arguments=None):
  parsed_arguments = parse_args(arguments)

  try:
    outcome = convert_pdf_to_markdown(
      parsed_arguments.input,
      parsed_arguments.output,
      force=parsed_arguments.force,
    )
  except Exception as error:
    print(f"❌ PDF to Markdown failed: {error}", file=sys.stderr, flush=True)
    return 1

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
