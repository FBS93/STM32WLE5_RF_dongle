# sys_usb_control_sequence_diagram

## Description

Dynamic view of a representative USB control exchange. The User sends a request through `sysc_hostCli` and `sysi_usbExternal`; `sysc_controller` processes it, then either sends no response or returns a response through the same interface and the User CLI.

## Diagram

```mermaid
sequenceDiagram
    actor User
    participant SYS_ARCH_COMPONENT_2 as sysc_hostCli
    participant SYS_ARCH_COMPONENT_1 as sysc_controller

    User->>SYS_ARCH_COMPONENT_2: Request
    %% SYS_ARCH_INTERFACE_1
    SYS_ARCH_COMPONENT_2->>SYS_ARCH_COMPONENT_1: sysi_usbExternal - Request
    SYS_ARCH_COMPONENT_1->>SYS_ARCH_COMPONENT_1: Process request
    alt Request without response
    else Request with response
        %% SYS_ARCH_INTERFACE_1
        SYS_ARCH_COMPONENT_1-->>SYS_ARCH_COMPONENT_2: sysi_usbExternal - Response
        SYS_ARCH_COMPONENT_2-->>User: Response
    end
```
