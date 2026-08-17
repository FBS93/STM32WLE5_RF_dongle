# sys_modes_state_diagram

## Description

This diagram is a compact abstraction of the RF-operation lifecycle with six architectural states: `INIT`, `IDLE`, `TX`, `RX`, `CAD`, and `CALIBRATION`. `INIT` is the internal startup phase; host commands are modeled from the host-visible states, and active operations return to `IDLE` when complete. `TX` groups the three transmit operation variants.

## State diagram

```mermaid
stateDiagram-v2
    [*] --> INIT: SYSTEM_START
    INIT --> IDLE: INITIALIZATION_COMPLETE

    IDLE --> IDLE: GET_CONFIG_REQUEST
    TX --> TX: GET_CONFIG_REQUEST
    RX --> RX: GET_CONFIG_REQUEST
    CAD --> CAD: GET_CONFIG_REQUEST
    CALIBRATION --> CALIBRATION: GET_CONFIG_REQUEST

    IDLE --> IDLE: SET_CONFIG
    TX --> IDLE: SET_CONFIG
    RX --> IDLE: SET_CONFIG
    CAD --> IDLE: SET_CONFIG
    CALIBRATION --> IDLE: SET_CONFIG

    IDLE --> CALIBRATION: CALIBRATE
    TX --> CALIBRATION: CALIBRATE
    RX --> CALIBRATION: CALIBRATE
    CAD --> CALIBRATION: CALIBRATE
    CALIBRATION --> CALIBRATION: CALIBRATE

    IDLE --> TX: TX_PACKET
    TX --> TX: TX_PACKET
    RX --> TX: TX_PACKET
    CAD --> TX: TX_PACKET
    CALIBRATION --> TX: TX_PACKET

    IDLE --> RX: RX_START
    TX --> RX: RX_START
    RX --> RX: RX_START
    CAD --> RX: RX_START
    CALIBRATION --> RX: RX_START

    IDLE --> CAD: CAD_START
    TX --> CAD: CAD_START
    RX --> CAD: CAD_START
    CAD --> CAD: CAD_START
    CALIBRATION --> CAD: CAD_START

    IDLE --> TX: TX_CONTINUOUS_WAVE
    TX --> TX: TX_CONTINUOUS_WAVE
    RX --> TX: TX_CONTINUOUS_WAVE
    CAD --> TX: TX_CONTINUOUS_WAVE
    CALIBRATION --> TX: TX_CONTINUOUS_WAVE

    IDLE --> TX: TX_CONTINUOUS_PREAMBLE
    TX --> TX: TX_CONTINUOUS_PREAMBLE
    RX --> TX: TX_CONTINUOUS_PREAMBLE
    CAD --> TX: TX_CONTINUOUS_PREAMBLE
    CALIBRATION --> TX: TX_CONTINUOUS_PREAMBLE

    IDLE --> IDLE: STOP
    TX --> IDLE: STOP
    RX --> IDLE: STOP
    CAD --> IDLE: STOP
    CALIBRATION --> IDLE: STOP

    IDLE --> IDLE: GET_DIAGNOSTICS_REQUEST
    TX --> TX: GET_DIAGNOSTICS_REQUEST
    RX --> RX: GET_DIAGNOSTICS_REQUEST
    CAD --> CAD: GET_DIAGNOSTICS_REQUEST
    CALIBRATION --> CALIBRATION: GET_DIAGNOSTICS_REQUEST

    TX --> IDLE: OPERATION_COMPLETE
    RX --> IDLE: OPERATION_COMPLETE
    CAD --> IDLE: OPERATION_COMPLETE
    CALIBRATION --> IDLE: OPERATION_COMPLETE
```

## States

| State | Description |
|---|---|
| `INIT` | Internal startup phase before the controller is ready for normal operation. MCU radio startup/cold-start calibration is included here as an implementation detail; it is not a host-visible command state. |
| `IDLE` | No host-visible RF operation is active. |
| `TX` | One transmit operation is active; packet and continuous transmit variants are abstracted by this mode. |
| `RX` | The configured receive operation is active. |
| `CAD` | The configured channel-activity-detection operation is active. |
| `CALIBRATION` | The explicit host radio-calibration operation is active. |

## Events

| Event | Description |
|---|---|
| `GET_CONFIG_REQUEST` | Reception on [SYS_ARCH_INTERFACE_1](sys_interfaces.md#sys_arch_interface_1) requests the current RF configuration; self-transition from each host-visible state. [SYS_REQ_INTERFACE_1](../req/sys_interfaces.md#get-config-request) |
| `SET_CONFIG` | Reception on [SYS_ARCH_INTERFACE_1](sys_interfaces.md#sys_arch_interface_1) applies the RF configuration; transition to `IDLE` from each active state and self-transition from `IDLE`. [SYS_REQ_INTERFACE_1](../req/sys_interfaces.md#set-config-request) |
| `CALIBRATE` | Reception on [SYS_ARCH_INTERFACE_1](sys_interfaces.md#sys_arch_interface_1) starts calibration in `CALIBRATION`, preempting any active state. [SYS_REQ_INTERFACE_1](../req/sys_interfaces.md#calibrate-request) |
| `TX_PACKET` | Reception on [SYS_ARCH_INTERFACE_1](sys_interfaces.md#sys_arch_interface_1) starts packet transmission in `TX`, preempting any active state. [SYS_REQ_INTERFACE_1](../req/sys_interfaces.md#tx-packet-request) |
| `RX_START` | Reception on [SYS_ARCH_INTERFACE_1](sys_interfaces.md#sys_arch_interface_1) starts the configured receive operation in `RX`, preempting any active state. [SYS_REQ_INTERFACE_1](../req/sys_interfaces.md#rx-start-request) |
| `CAD_START` | Reception on [SYS_ARCH_INTERFACE_1](sys_interfaces.md#sys_arch_interface_1) starts the configured CAD operation in `CAD`, preempting any active state. [SYS_REQ_INTERFACE_1](../req/sys_interfaces.md#cad-start-request) |
| `TX_CONTINUOUS_WAVE` | Reception on [SYS_ARCH_INTERFACE_1](sys_interfaces.md#sys_arch_interface_1) starts continuous-wave transmission in `TX`, preempting any active state. [SYS_REQ_INTERFACE_1](../req/sys_interfaces.md#tx-continuous-wave-request) |
| `TX_CONTINUOUS_PREAMBLE` | Reception on [SYS_ARCH_INTERFACE_1](sys_interfaces.md#sys_arch_interface_1) starts infinite-preamble transmission in `TX`, preempting any active state. [SYS_REQ_INTERFACE_1](../req/sys_interfaces.md#tx-continuous-preamble-request) |
| `STOP` | Reception on [SYS_ARCH_INTERFACE_1](sys_interfaces.md#sys_arch_interface_1) terminates the active RF operation; transition to `IDLE` from each active state and self-transition from `IDLE`. [SYS_REQ_INTERFACE_1](../req/sys_interfaces.md#stop-request) |
| `GET_DIAGNOSTICS_REQUEST` | Reception on [SYS_ARCH_INTERFACE_1](sys_interfaces.md#sys_arch_interface_1) retrieves diagnostics without changing the mode; self-transition from each host-visible state. [SYS_REQ_INTERFACE_1](../req/sys_interfaces.md#get-diagnostics-request) |
| `SYSTEM_START` | Internal system-start event entering the controller startup phase. [SYS_ARCH_DESIGN_1](sys_designs.md#sys_arch_design_1) |
| `INITIALIZATION_COMPLETE` | Internal startup-completion event indicating that the controller is ready for normal operation. [SYS_ARCH_DESIGN_1](sys_designs.md#sys_arch_design_1) |
| `OPERATION_COMPLETE` | Internal operation-completion event returning `TX`, `RX`, `CAD`, or `CALIBRATION` to `IDLE`; RX and CAD lifecycle behavior follows [RX operation](../req/sys_interfaces.md#rx-operation) and [CAD operation](../req/sys_interfaces.md#cad-operation). [SYS_ARCH_DESIGN_1](sys_designs.md#sys_arch_design_1) |
