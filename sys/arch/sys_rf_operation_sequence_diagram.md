# sys_rf_operation_sequence_diagram

## Description

Dynamic view of a representative RF-operation exchange. The sequence traces the User request through `sysc_hostCli` and `sysi_usbExternal`, `sysc_controller` processing, and the mutually exclusive TX/RX interaction through `sysi_rfExternal`. TX has no operation response; RX returns its result or response to the User through `sysi_usbExternal` and the User CLI.

## Diagram

```mermaid
sequenceDiagram
    actor User
    participant SYS_ARCH_COMPONENT_2 as sysc_hostCli
    participant SYS_ARCH_COMPONENT_1 as sysc_controller
    participant RFENV as RF environment

    User->>SYS_ARCH_COMPONENT_2: RF operation request
    %% SYS_ARCH_INTERFACE_1
    SYS_ARCH_COMPONENT_2->>SYS_ARCH_COMPONENT_1: sysi_usbExternal - RF operation request
    SYS_ARCH_COMPONENT_1->>SYS_ARCH_COMPONENT_1: Process request
    alt TX
        %% SYS_ARCH_INTERFACE_2
        SYS_ARCH_COMPONENT_1->>RFENV: sysi_rfExternal - TX
    else RX
        %% SYS_ARCH_INTERFACE_2
        RFENV->>SYS_ARCH_COMPONENT_1: sysi_rfExternal - RX
        %% SYS_ARCH_INTERFACE_1
        SYS_ARCH_COMPONENT_1-->>SYS_ARCH_COMPONENT_2: sysi_usbExternal - RX response
        SYS_ARCH_COMPONENT_2-->>User: RX response
    end
```
