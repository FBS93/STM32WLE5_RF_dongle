# Getting started

## Overview

This document describes the shortest path to start working with the repository.

The expected development entry point is VS Code running the provided devcontainer.

## Host prerequisites

The host environment shall provide the following:
- VS Code with the `ms-vscode-remote.vscode-remote-extensionpack` extension installed.
- A Git identity configured. The `ms-vscode-remote.remote-containers` extension copies the host `.gitconfig` into the container at startup; see [Sharing Git credentials with your container](https://code.visualstudio.com/remote/advancedcontainers/sharing-git-credentials).
- OpenCode installed. The devcontainer persists OpenCode state (including credentials and sessions) through the bind mount defined in [devcontainer.json](../../.devcontainer/devcontainer.json).
- Host GUI support for containerized graphical applications.

## Initial setup

1. If EW is used as the template for a derived project, create the derived repository first.
2. Open the repository in VS Code.
3. Run `Dev Containers: Reopen in Container` from the VS Code command palette.
4. Wait until the devcontainer build and startup process completes.
5. Review project-specific settings exposed through [devcontainer.json](../../.devcontainer/devcontainer.json).
6. Review template update behavior in [template_sync.md](template_sync.md) before changing template-owned files or adding project-specific exclusions to [.templatesyncignore](../../.templatesyncignore).
7. Apply the following GitHub repository configuration:
  - `Settings > Actions > General > Workflow permissions > Read and write permissions`
  - `Settings > Actions > General > Pull request workflows > Allow GitHub Actions to create and approve pull requests`
8. See [vscode_workflow.md](vscode_workflow.md) for repository task entry points.
9. Use the [AI-assisted development workflow](ai_assisted_development_workflow.md) and [V-model methodology](../development_methodology/development_methodology_overview.md) as the entry points for generating project-specific engineering work products.
