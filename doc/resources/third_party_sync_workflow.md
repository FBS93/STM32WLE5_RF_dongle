# Third-party sync workflow

## Overview

This document describes the GitHub Actions workflow that automates third-party repositories synchronization.

The workflow definition is implemented in [../../.github/workflows/third_party_sync.yml](../../.github/workflows/third_party_sync.yml). The underlying synchronization script is documented in [../../tools/third_party_sync/third_party_sync.md](../../tools/third_party_sync/third_party_sync.md).

## Prerequisites

The `third_party_sync.yml` workflow creates branches and pull requests automatically. To allow this behavior, apply the following settings in the GitHub repository:

- `Settings > Actions > General > Workflow permissions > Read and write permissions`
- `Settings > Actions > General > Pull request workflows > Allow GitHub Actions to create and approve pull requests`
