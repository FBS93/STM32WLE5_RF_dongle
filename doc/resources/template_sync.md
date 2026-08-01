# Template sync

## Overview

This template is intended to be used as the source repository for derived embedded projects.

The repository includes the [actions-template-sync](https://github.com/AndreasAugustin/actions-template-sync) workflow out of the box through the [`template_sync.yml`](../../.github/workflows/template_sync.yml). This enables derived repositories to periodically pull updates from the template repository and automatically open synchronization pull requests.

Files excluded through `.templatesyncignore` will no longer receive updates from the template repository. If a derived repository needs to exclude project-specific files or directories from synchronization, this file can be modified accordingly.
