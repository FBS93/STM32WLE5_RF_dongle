# AI-assisted development workflow

## Overview

AI-assisted development defines a workflow in which the repository development methodology and work products guide AI-assisted design, implementation and validation tasks. In this template, it is integrated with the V-model development methodology and supporting technical skills as follows:
1. The [V-model development methodology](../development_methodology/development_methodology_overview.md) defines the engineering processes, their inputs, outputs, resources and traceability rules.
2. The [skills](../../.opencode/skills/) provided by the repository map V-model processes and supporting technical tasks to AI-usable entry points.
3. [OpenCode](https://opencode.ai/) provides the project-local AI environment, including the custom Embedded Workbench orchestrator and its skills.
4. The custom Embedded Workbench orchestrator coordinates delegated execution.

The AI workflow runs inside the repository devcontainer. This constrains its normal scope of action to the repository workspace and to the integrations explicitly exposed by the devcontainer configuration, which improves safety compared to running an AI model directly on the host environment.

## Glossary

| Term | Definition |
|------|------------|
| AI-assisted development | Development approach where repository documentation, process skills and work products guide AI-assisted engineering activities. |

## Usage example

- A repository activity is aligned first with the corresponding V-model process, if one applies, as defined in the development methodology.
- Otherwise, the smallest supporting repository skill matching the technical activity is used.
- The selected skill references the applicable process documentation and resources, allowing the corresponding work products to be produced or reviewed in a repository-aligned and traceable way.
