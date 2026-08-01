# sys-architecture

## Purpose

Define system architecture from system requirements.

## Input work products

- System requirements

## Output work products

- System architecture

## Steps

1. Review the system requirements.
2. Identify the system architecture elements required to address the system requirements.
3. Define the system architecture based on the identified system architecture elements.
4. Define traceability between system architecture elements and system requirements.
5. Check the system architecture for completeness, consistency and correctness.

## Guidelines

### System architecture work product

The system architecture work product shall follow the [Architecture definition](../../resources/architecture_definition.md).

#### System Modes

The system architecture shall define the system modes as a system design. 

The system modes system design shall:
- Be defined based on the system requirements and shall describe the system’s dynamic behavior in terms of states, the features available in each state and the events that trigger transitions between them. If system modes are not fully specified in the system requirements, a basic and as simple as possible set of system modes shall be assumed.
- Be represented using a state diagram. 
  - States and events defined in the state diagram are not considered architecture traceable elements. The complete system modes shall be treated as a single traceable system design.
  - Events triggering state transitions shall correspond to real interactions defined in the system requirements and system architecture:
    - Events originating from interfaces shall reference the corresponding interface identifiers and specify the expected reception that triggers the transition.
    - Internal events (e.g., faults, interrupts, ...) shall be aligned with and reference the corresponding system design where applicable.
- Be defined in a dedicated Markdown document named `sys_modes_state_diagram.md`.

Each state and event defined in the state diagram shall be explicitly described in dedicated tables within the same document. The following document template shall be used:

```md
# sys_modes_state_diagram

## Description

<description>

## State diagram

<mermaid diagram definition>

## States

| State | Description |
|---|---|
| <STATE_NAME> | <DESCRIPTION> |

## Events

| Event | Description |
|---|---|
| <EVENT_NAME> | <DESCRIPTION> |
```