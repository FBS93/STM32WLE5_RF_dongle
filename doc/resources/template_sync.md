# Template sync

## Overview

This template is intended to be used as the source repository for derived embedded projects.

The repository includes the [actions-template-sync](https://github.com/AndreasAugustin/actions-template-sync) workflow out of the box through the [`template_sync.yml`](../../.github/workflows/template_sync.yml). This enables derived repositories to periodically pull updates from the template repository and automatically open synchronization pull requests.

Files excluded through [`.templatesyncignore`](../../.templatesyncignore) will no longer receive updates from the template repository. If a derived repository needs to exclude project-specific files or directories from synchronization, this file can be modified accordingly.

## GitHub Actions workflow updates

Template sync needs `TEMPLATE_SYNC_PAT` only when it shall update GitHub Actions workflow files. Otherwise, this GitHub secret may be omitted.

1. In GitHub, open the derived repository owner's `Settings > Developer settings > Personal access tokens > Tokens (classic)`.
2. Select `Generate new token (classic)`.
3. In `Note`, enter a descriptive name, for example: `Template sync workflow updates`.
4. Select the expiration date.
5. Select these scopes:
  - `repo`
  - `workflow`
  - `read:org`
6. Generate the token.
7. In the derived repository, open `Settings > Secrets and variables > Actions`.
8. Create a repository secret named `TEMPLATE_SYNC_PAT` and save the generated token as its value.

If synchronization of GitHub Actions workflow files is not wanted, exclude them using [`.templatesyncignore`](../../.templatesyncignore).
