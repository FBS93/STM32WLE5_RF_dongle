"""Shared Docgen test runner."""

import subprocess
import sys
from pathlib import Path

DOCGEN_PATH = Path(__file__).parents[1] / "docgen.py"


def run_docgen(repository_root, output_path, *arguments, cwd=None, env=None):
  return subprocess.run(
    [
      sys.executable,
      str(DOCGEN_PATH),
      "--root",
      str(repository_root),
      "--output",
      str(output_path),
      *arguments,
    ],
    check=False,
    text=True,
    capture_output=True,
    cwd=cwd,
    env=env,
  )
