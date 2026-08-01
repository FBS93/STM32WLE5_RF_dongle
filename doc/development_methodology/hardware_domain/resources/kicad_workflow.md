# KiCad workflow

## Overview

This document describes the KiCad state persistence strategy used by the hardware domain to keep the KiCad user environment reproducible inside the devcontainer.

See [vscode_workflow.md](../../../resources/vscode_workflow.md) for repository task entry points.

## State persistence strategy

KiCad uses environment variables to configure the paths where it reads and writes user state. These paths are configured in [.devcontainer/devcontainer.json](../../../../.devcontainer/devcontainer.json):

`KICAD_CONFIG_HOME` and `KICAD_DOCUMENTS_HOME` store KiCad configuration and user documents respectively. These paths point to a Docker volume that is not versioned with the workspace. The state tooling can capture the current runtime state and restore a versioned state when one is available. This avoids versioning volatile metadata and other files that change frequently during normal KiCad usage and would otherwise add noise to the repository. The scripts are documented in [tools/kicad_state/kicad_state.md](../../../../tools/kicad_state/kicad_state.md).

`KICAD10_3RD_PARTY` points to a workspace-local location for future third-party libraries, plugins and related resources.

The persistence flow used to keep the KiCad environment stable across fresh clones and devcontainer rebuilds is:
1. The devcontainer `postStartCommand` runs [`post_start.sh`](../../../../.devcontainer/post_start.sh).
2. [`post_start.sh`](../../../../.devcontainer/post_start.sh) prepares the persistent volume mounted at `/kicad-runtime`.
3. [`post_start.sh`](../../../../.devcontainer/post_start.sh) runs [`restore_kicad_state.py`](../../../../tools/kicad_state/restore_kicad_state.py).
4. [`restore_kicad_state.py`](../../../../tools/kicad_state/restore_kicad_state.py) restores a versioned state only when one is available and the runtime is empty.
5. KiCad then works directly against the persistent runtime paths.

## Notes

- The automatic restore happens only on container start.
- The runtime is not overwritten automatically once it already contains content.
