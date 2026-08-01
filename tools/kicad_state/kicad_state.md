# KiCad state overview

This document describes the tool scripts located in tools/kicad_state/, which are listed below:
- `restore_kicad_state.py` restores `tools/kicad_state/state/<version>/` into the configured KiCad runtime paths. By default it restores only when the runtime is empty. Use `--force` to replace the current runtime.
- `save_kicad_state.py` captures the current KiCad runtime into `tools/kicad_state/state/<version>/`.
- `kicad_state_common.py` provides shared helpers for path resolution, version detection, directory replacement and cleanup.

# Glossary

| Term | Definition |
|---|---|
| | |

# Usage example

Restore the current KiCad runtime from the stored state:

```bash
python tools/kicad_state/restore_kicad_state.py
```

Force-reset the runtime back to the stored state:

```bash
python tools/kicad_state/restore_kicad_state.py --force
```

Capture the current KiCad runtime back into the repository state:

```bash
python tools/kicad_state/save_kicad_state.py
```
