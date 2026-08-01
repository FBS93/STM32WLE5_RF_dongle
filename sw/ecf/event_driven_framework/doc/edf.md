# Event Driven Framework (EDF) overview

EDF is a lightweight, multi-platform event-driven framework designed to run on both embedded systems and host environments. Its purpose is to provide:
- A structured event-driven architecture.
- Active Objects implemented as event consumers using Hierarchical State Machines (HSM) for scalable and maintainable application logic.
- A unified event management and dispatching mechanism based on a publish-subscribe model.
- Static event pools for deterministic and efficient memory management of mutable events.
- Deterministic real-time scheduling kernels.
- Configurable framework parametrization to adapt to resource-constrained systems.

All configuration options for this library are documented in the "EDF default configuration" section of [CMakeLists.txt](../../../../CMakeLists.txt).

Core-specific notes currently documented:
- [ARM Cortex-M EDF core](../src/core/baremetal/arm_cortex_m/doc/edf_core.md)

This framework is a derivative work based on:
- QP/C (c) Quantum Leaps, LLC.

# Glossary

| Term | Definition |
|------|------------|
| Active Object | Event-driven software component encapsulating behavior implemented as a HSM.|
| HSM | Hierarchical State Machine. |
| Publish-subscribe | Event distribution mechanism decoupling event producers from event consumers. |
| Event Pool | Section of static memory used for deterministic and safe sharing of mutable events. |
| Immutable Event | Read-only event instance that cannot be changed at runtime and can be safely shared without memory management. |
| Mutable Event | Event instance created at runtime whose payload is filled dynamically and managed through static event pools. |

# Startup contract for event sources

EDF requires that no EDF events are published before the user-specific startup callback `EDF_onStartup()` is executed. This restriction applies to both synchronous user code and asynchronous contexts (e.g., ISRs, threads).

The following startup sequence shall be applied:
1. Call `EDF_init()`.
2. Initialize EDF structures used by the application (publish/subscribe mechanisms, active objects, pools, time events, ...).
3. Do not initialize/enable asynchronous contexts (e.g., ISRs, threads) yet if they can publish EDF events.
4. Call `EDF_run()`.
5. The initialization and enabling of asynchronous event sources (e.g., ISRs) shall only occur at the end of `EDF_onStartup()` user callback implementation or later.
6. Return from `EDF_onStartup()` quickly (avoid long blocking execution to prevent event queue overflows from asynchronous event sources).

This sequence avoids publishing events while EDF startup is still in progress and reduces the risk of queue overflow during startup. However, the user remains responsible for sizing the event queues accordingly to accommodate any accumulation of events that may occur during the startup phase.

# Shutdown contract for `EDF_onShutdown()`

`EDF_onShutdown()` is a termination hook. In this callback, EDF API availability is not guaranteed across all ports and shutdown paths. For portability, the callback shall be treated as a system/application teardown point, not as a normal EDF execution context.

The following shutdown rules shall be applied:
1. Do not call EDF APIs from `EDF_onShutdown()` (e.g., posting/publishing events, active object start/subscribe operations, time-event APIs, ...).
2. Use `EDF_onShutdown()` only to stop external sources and integrations (peripherals, sockets, ...).
3. Keep the callback short and deterministic.
4. If a complex shutdown sequence is required, trigger it from a normal execution context before `EDF_onShutdown()` is executed.

This contract prevents non-portable behavior and avoids shutdown-time race conditions caused by assuming EDF is still fully available during termination.

# Fuzzing port contract

ECF provides the port `EBF_PORT_POSIX_NON_PREEMPTIVE_FUZZ` to fuzz EDF applications automatically without requiring the user to deal with [AFL++](https://github.com/AFLplusplus/AFLplusplus) integration details. The EDF application shall only define one fuzzable-event table by including `edf_fuzz.h` and using the following macros:
  - `EDF_FUZZ_EVENT_TABLE(...)`
  - `EDF_FUZZ_IMMUTABLE_EVENT(sig, event_ptr)`
  - `EDF_FUZZ_MUTABLE_EVENT(sig, event_size)`

This registers the events that shall be fuzzed. Once the fuzzing port is selected, EDF fuzzes them automatically. The high-level flow for each fuzzing input is the following:
- The port reads one fuzz input from standard input and interprets it as a sequence of event segments.
- Each segment starts with a selector encoded in little-endian with `sizeof(EDF_event_signal_t)` bytes. The selector is mapped to one entry of the application-defined fuzzable-event table using modulo over the table length.
  - Immutable entries publish the provided event instance directly.
  - Mutable entries define the full event size, including `EDF_event_t` as the first member and the next bytes of the segment are interpreted as the mutable event payload with length `event_size - sizeof(EDF_event_t)`. If a mutable segment is truncated, that root event is skipped and testcase processing stops at that segment.
- EDF injects the selected fuzzed event and executes the kernel until quiescence before advancing to the next fuzzed event.

Important notes:
- The port owns `EAF_onError()`. User application code shall not redefine `EAF_onError()` for this port (e.g., guard another implementation with conditional compilation on `EBF_PORT`). In fuzzing mode, controlled asserts are treated as defined behavior, so the port logs the error location and terminates the testcase with `exit(0)` instead of surfacing them as AFL++ crashes.
- `ECF_TEST=ON` is not supported with the `EBF_PORT_POSIX_NON_PREEMPTIVE_FUZZ` port.
- To enable the ECF fuzzing example, set `EDF_EXAMPLE_FUZZ=ON`, `EBF_CORE=EBF_CORE_OS` and `EBF_PORT=EBF_PORT_POSIX_NON_PREEMPTIVE_FUZZ`.
- The devcontainer fuzzing integration approach prioritizes zero host configuration so the setup remains plug-and-play. For this reason it exports `AFL_SKIP_CPUFREQ=1` to skip the host CPU frequency governor check, `AFL_NO_AFFINITY=1` to disable AFL++ CPU pinning and `AFL_I_DONT_CARE_ABOUT_MISSING_CRASHES=1` to bypass the `core_pattern` sanity check. These settings improve portability inside the container but can hide host-side performance issues or reduce fuzzing throughput. If better results are needed, review these variables and tune the host accordingly.

# Usage example

See complete examples in:
- [Event delivery test](../test/event_delivery/doc/test_event_delivery.md)
- [HSM test](../test/hsm/doc/test_hsm.md)
- [Time event test](../test/time_event/doc/test_time_event.md)
- [Fuzz example](../examples/fuzz_example/doc/fuzz_example.md)
