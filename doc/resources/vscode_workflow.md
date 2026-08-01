# VS Code workflow

## Overview

VS Code running the provided devcontainer is the official development entry point for the repository.

The available VS Code functions are organized by domain. Each function is listed in this document, while the corresponding domain-specific documentation remains the source of truth for its detailed usage.

The repository exposes these functions through three VS Code entry point types:
- Tasks defined in [tasks.json](../../.vscode/tasks.json).
- Launch configurations defined in [launch.json](../../.vscode/launch.json).
- Task buttons configured in [settings.json](../../.vscode/settings.json), grouped as workbench and software toolboxes.

## Workbench

| Function | VS Code entry point | Purpose | Reference |
|---|---|---|---|
| Raspberry Pi SSH access | `connect_rpi_ssh` task, `🔗 Connect Raspberry Pi (SSH)` task button | Opens an SSH session to the Raspberry Pi used by the repository workflow. | [Raspberry Pi setup](raspberry_pi_setup.md) |
| OpenCode web GUI | `run_opencode_web_gui` task, `🌐 OpenCode web GUI` task button | Opens the OpenCode web interface for repository AI workflows on port `4096`. | [AI-assisted development workflow](ai_assisted_development_workflow.md) |
| PulseView web | `run_pulseview_web` task, `📈 Run PulseView web` task button | Starts the remote PulseView web visualization workflow on the Raspberry Pi and prints the browser URL. | [Embedded target remote logic analyzer](../development_methodology/resources/embedded_target_remote_logic_analyzer.md) |
| Logic analyzer capture sync | `sync_logic_analyzer_captures` task, `📥 Sync logic analyzer captures` task button | Mirrors `LOGIC_ANALYZER_ARTIFACTS_DIR` from the workspace to the same relative path under `/tmp/` on the Raspberry Pi. | [Embedded target remote logic analyzer](../development_methodology/resources/embedded_target_remote_logic_analyzer.md) |
| Third-party sync | `run_third_party_sync` task, `🔄 Sync third-party repositories` task button | Runs the local third-party synchronization workflow. | [Third-party sync workflow](third_party_sync_workflow.md), [third_party_sync tool](../../tools/third_party_sync/third_party_sync.md) |
| Documentation generation | `run_docgen` task, `📄 Generate documentation` task button | Generates static repository documentation in the configured `DOCGEN_OUTPUT` path. | [docgen tool](../../tools/docgen/docgen.md) |
| Release validation | `validate_release` task, `☑️ Validate release` task button | Runs configured CI and generates release documentation without creating or publishing a release, so the release validation can be checked before publication. | [CI](ci.md) |
| Release | `run_release` task, `🚀 Run release` task button | Generates a release. | [CI](ci.md) |

## Software domain

| Function | VS Code entry point | Purpose | Reference |
|---|---|---|---|
| Build | `build` task, `🔨 Build` task button | Builds the selected software target using the active CMake preset and active CMake build target. | |
| Clean | `clean` task, `🧹 Clean` task button | Cleans the build artifacts generated for the active CMake preset. | |
| Format | `format` task, `🧼 Format all source files` task button | Formats repository C, ASM, CMake and Python sources using the configured repository tooling. ASM files are also formatted on save through VS Code settings. | [Embedded C coding guidelines](../development_methodology/software_domain/resources/embedded_c_coding_guidelines.md), [Python coding guidelines](../development_methodology/resources/python_coding_guidelines.md), [ASM formatter tool](../development_methodology/software_domain/resources/assembler_format_tool.md) |
| Lint | `lint` task, `🔎 Run linter` task button | Runs the configured software static analysis workflow using the active CMake preset. | |
| CI | `run_ci` task, `🚦 Run CI` task button | Runs CI manually for the active CMake preset. | [CI](ci.md) |
| Fuzz | `run_fuzz` task, `🧪 Run fuzzer` task button | Runs the selected fuzz target using the active CMake build target. | |
| Fuzz source coverage | `run_fuzz_coverage` task, `📊 Run fuzzer coverage` task button | Replays the fuzz campaign results available for the selected fuzz target to generate coverage reports using the active CMake build target. | [Fuzz example](../../sw/ecf/event_driven_framework/examples/fuzz_example/doc/fuzz_example.md) |
| Host debug | `Host` launch configuration | Launches host-side debugging for the active CMake launch target. | |
| Target debug | `Target STM32F103C8Tx (launch/attach)` launch configurations, `run_target_gdb_server` task | Launches or attaches a remote debug session on the target platform. | [Embedded target remote debugging](../development_methodology/software_domain/resources/embedded_target_remote_debugging.md) |
| Target logging | `run_target_logging_server` task, `📃 Enable target logging` task button | Starts the remote logging workflow for the target platform. | [Embedded target remote logging](../development_methodology/software_domain/resources/embedded_target_remote_logging.md) |

## Expected extensions

The following VS Code extensions are declared for the devcontainer in [devcontainer.json](../../.devcontainer/devcontainer.json).

The repository also expects VS Code Remote - Containers support on the host machine in order to open the workspace through the provided devcontainer.

### Host prerequisite

- `ms-vscode-remote.vscode-remote-extensionpack`

### Devcontainer - Workbench
- `spencerwmiles.vscode-task-buttons`
- `mhutchie.git-graph`
- `bierner.markdown-preview-github-styles`
- `ms-vscode.live-server`

### Devcontainer - Software domain
- `ms-vscode.cmake-tools`
- `ms-vscode.cpptools`
- `dan-c-underwood.arm`
- `ms-python.python`
- `marus25.cortex-debug`
- `ms-vscode.hexeditor`
- `ms-vscode.vscode-serial-monitor`
- `emeraldwalk.runonsave`

### Devcontainer - Hardware domain

No additional VS Code extension is currently declared specifically for the repository hardware workflow beyond the common extensions and the tools provided by the devcontainer.
