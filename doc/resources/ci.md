# CI

## Overview

This document describes how the CI workflow of EW validates releases and changes before they are pushed to configured remote branches.

See [vscode_workflow.md](vscode_workflow.md) for repository task entry points.

The CI workflow uses [Docgen tool](../../tools/docgen/docgen.md) to generate documentation. See [automatic_documentation_generation.md](automatic_documentation_generation.md).

### Release versioning

Release tags use the `vMAJOR.MINOR.PATCH` format:

- `MAJOR`: Significant changes or milestones.
- `MINOR`: Incremental features or improvements.
- `PATCH`: Bug fixes.

## Architecture

- The pre-push hook runs CI for non-deletion updates to configured remote branches, validating each configured CMake preset.
- The `🚦 Run CI` task button validates the active CMake configure preset on demand.
- The `☑️ Validate release` task button requests a `vMAJOR.MINOR.PATCH` version, validates every preset in `CI_PRESETS` and generates release documentation.
- The `🚀 Release` task button validates a release, creates and pushes its tag, and publishes the generated documentation.

Git push --> Pre-push hook --> CI runner --> Configured CMake presets

VS Code `🚦 Run CI` task button --> CI runner --> Active CMake configure preset

VS Code `☑️ Validate release` task button --> CI runner --> Configured CMake presets + Docgen

VS Code `🚀 Release` task button --> Release runner --> Release tag + upload generated documentation to GitHub Pages

## CI Setup

CI for requested CMake presets is executed by [run_ci.sh](../../.vscode/tasks/run_ci.sh). This script:
- Checks formatting without modifying source files.
- Configures, builds, and lint-checks each requested preset.
- Runs CTest for each requested preset.

Configured CI is executed by [run_configured_ci.sh](../../.vscode/tasks/run_configured_ci.sh). This script:
- Runs CI for every preset listed in `CI_PRESETS`.

Matching remote branch updates are checked by the [pre-push hook](../../.githooks/pre-push). This script:
- Runs configured CI from the current working tree when a non-deletion update targets a branch listed in `CI_BRANCHES`.
- Skips CI for other updates; it remains a local safeguard that can be bypassed with `git push --no-verify`.

Release validation is performed by [validate_release.sh](../../.vscode/tasks/validate_release.sh). This script:
- Validates a `vMAJOR.MINOR.PATCH` release version.
- Runs configured CI and generates release documentation with its CI result.

Release publication is performed by [run_release.sh](../../.vscode/tasks/run_release.sh). This script:
- Validates the release and creates and pushes its `vMAJOR.MINOR.PATCH` tag.
- Publishes documentation generated from each tagged commit under `gh-pages/vMAJOR.MINOR.PATCH/`.
- Regenerates the root Pages index with links to the published release versions.

CI configured presets containing Hardware-in-the-Loop (HiL) tests require the hardware setup to be configured and operational. See [Raspberry Pi setup](raspberry_pi_setup.md) and [dual targeting setup](dual_targeting_setup.md).

## Dependencies

### Devcontainer

Make sure that the environment variables in [devcontainer.json](../../.devcontainer/devcontainer.json) are configured for CI:

- `CI_BRANCHES` lists the exact remote branch names, separated by spaces, that trigger pre-push CI.
- `CI_PRESETS` lists the CMake presets, separated by spaces, that configured CI validates for pre-push and release.

The devcontainer configures the [pre-push hook](../../.githooks/pre-push) through `core.hooksPath` when the container is created. See [devcontainer.json](../../.devcontainer/devcontainer.json)

### GitHub Pages

After the first successful release creates and publishes the `gh-pages` branch, a repository administrator shall then configure GitHub Pages once:

1. Open the GitHub repository **Settings** page.
2. Select **Pages**.
3. Under **Build and deployment**, select **Deploy from a branch** as the source.
4. Select the `gh-pages` branch and the `/(root)` folder.
5. Save the configuration.

Unless a custom domain is configured, GitHub Pages publishes the release index at `https://<owner>.github.io/<repository>/`. A specific release is available at `https://<owner>.github.io/<repository>/vMAJOR.MINOR.PATCH/`.
