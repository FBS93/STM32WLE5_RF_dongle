# Embedded target remote logic analyzer

## Overview

This document describes how to use a remote logic analyzer connected to a Raspberry Pi.

The Raspberry Pi remains the hardware gateway. Physical logic analyzer access is performed only on the Raspberry Pi, while the devcontainer triggers remote Sigrok commands over SSH and exchanges artifacts through SCP. From the devcontainer, it is also possible to start a web interface on the Raspberry Pi that provides access to the PulseView instance running on the Raspberry Pi in order to control the logic analyzer in real time or inspect previously captured traces.

## Architecture

- Embedded target signals are connected to a logic analyzer.
- The logic analyzer is connected to the Raspberry Pi via USB.
- The Raspberry Pi runs `sigrok-cli` for capture and PulseView for visualization inside a virtual display.
- The devcontainer interacts with the Raspberry Pi through SSH and SCP.

Devcontainer --> SSH --> Raspberry Pi --> Logic analyzer --> Target signals

Browser --> noVNC/websockify --> TigerVNC --> Xvfb + Openbox --> PulseView

## Logic analyzer setup

The following tools and scripts are provided for the remote logic analyzer workflow:

- [Sigrok remote client](../../../tools/sigrok_remote_client/sigrok_remote_client.md) is a minimal Python wrapper for automation. It forwards all Sigrok commands directly to `sigrok-cli` and only adds remote execution, the explicit `LOGIC_ANALYZER_DEVICE` selector, and local artifact storage helpers.
- `.vscode/tasks/sync_logic_analyzer_captures.sh` mirrors `LOGIC_ANALYZER_ARTIFACTS_DIR` from the workspace to the same relative path under `/tmp/` on the Raspberry Pi so the current local artifacts can be viewed through PulseView web.
- `.vscode/tasks/run_pulseview_web.sh` starts a minimal web visualization stack on the Raspberry Pi using the `LOGIC_ANALYZER_DEVICE` selector so the logic analyzer can be used in real time and previously captured traces can also be inspected.

## Dependencies

Install the required packages on the Raspberry Pi:

```bash
sudo apt-get update
sudo apt-get install -y \
  sigrok-cli \
  pulseview \
  xvfb \
  openbox \
  tigervnc-scraping-server \
  novnc \
  websockify \
  wmctrl
```

Verify the installation:

```bash
sigrok-cli --version
pulseview --version
Xvfb -version
openbox --version
x0tigervncserver -version
websockify --version
wmctrl -V
```

These packages provide the base workflow, but some Sigrok-compatible devices also require hardware-specific runtime resources such as firmware files, udev rules, or vendor-side helpers before they can be opened successfully.

Example: `fx2lafw`-based logic analyzers such as many Saleae Logic-compatible devices require the matching `fx2lafw` firmware file to be available on the Raspberry Pi. If it is missing, Sigrok or PulseView can fail.

To diagnose this case on the Raspberry Pi, execute the following command:

```bash
find /usr /usr/local -name 'fx2lafw-saleae-logic.fw' 2>/dev/null
```

If the firmware file is missing, install the `fx2lafw` firmware resources for Sigrok on the Raspberry Pi before retrying the workflow. For this specific case on Raspberry Pi OS:

```bash
sudo apt-get update
sudo apt-get install -y sigrok-firmware-fx2lafw
find /usr /usr/local -name 'fx2lafw-saleae-logic.fw' 2>/dev/null
```

Make sure that the environment variables in [devcontainer.json](../../../.devcontainer/devcontainer.json) are configured for the logic analyzer setup and target environment:

- `RPI_USER` and `RPI_HOST` must match the Raspberry Pi SSH credentials.
- `LOGIC_ANALYZER_DEVICE` must match the Sigrok device selector assigned to the logic analyzer.
- `LOGIC_ANALYZER_ARTIFACTS_DIR` must point to the directory where fetched artifacts should be stored.

To discover `LOGIC_ANALYZER_DEVICE` on the Raspberry Pi, ask Sigrok to scan the connected devices:

```bash
sigrok-cli --scan
```

Copy the device selector shown at the start of the matching scan line, that is, the text before ` - `.

Examples:

```text
fx2lafw - Saleae Logic with 8 channels: D0 D1 D2 D3 D4 D5 D6 D7
```

Use:

```text
LOGIC_ANALYZER_DEVICE=fx2lafw
```

If Sigrok reports a more specific selector such as:

```text
fx2lafw:conn=1.34 - Saleae Logic with 8 channels: D0 D1 D2 D3 D4 D5 D6 D7
```

use the full selector:

```text
LOGIC_ANALYZER_DEVICE=fx2lafw:conn=1.34
```

This matches the official Sigrok and PulseView command-line model: `sigrok-cli --driver <selector>` and `pulseview -d <selector>` both expect the same device selector format.
