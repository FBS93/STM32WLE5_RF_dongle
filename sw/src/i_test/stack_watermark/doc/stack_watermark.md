# SW_INTEGRATION_TEST_10

## stack_watermark overview

Measure the product MSP stack watermark through J-Link/SWD while the public [CLI](../../../../cli/cli.md) supplies bounded, non-transmitting RF operations.

Upstream traceability:
- [SW_ARCH_DESIGN_2](../../../../arch/sw_designs.md#sw_arch_design_2)

Status: approved

## Use case

The product ELF is loaded and verified on the target through J-Link/SWD using the existing [remote debug setup](../../../../../doc/development_methodology/software_domain/resources/embedded_target_remote_debugging.md), without firmware instrumentation or an EDFTest image. At the exact `main` entry before its prologue, the execution context and ELF stack bounds are checked and only `[bottom, MSP)` is painted and read back.

Without intermediate halts, the public CLI drives three cycles of LoRa and FSK configuration, configuration queries, finite single RX, STOP and calibration, confirming public RF idle after each operation. No TX command is issued. A final STOP and idle confirmation precede the halt and capture of MSP and the complete stack. Teardown resumes and disconnects the product; failure recovery attempts STOP/idle, then reset and an idle check if needed, without masking the failure.

The CLI uses the first sorted FT230X product VCP by-id path on the Raspberry Pi, not the FT232 logging UART; no matches fail before programming. Other debug/UART clients must be inactive: cooperative locks and busy-client checks do not reserve hardware against IDE clients. @todo: assign specific ports in the test setup.

## Verification scope

Report capacity, observed reserved/overwritten bytes, free bytes and free percentage. Require at least `MIN_FREE_PERCENT` (30%) free: this is a test acceptance criterion, not a new architecture requirement. Capacity comes from ELF symbols, not a hardcoded stack size. Count the initially unpainted upper stack and the final MSP reservation as used, even if bytes match the pattern; the remaining untouched prefix is the measured free margin.

Watermarking cannot detect transient reservations without writes or writes identical to the pattern. Startup before `main` is excluded. This sequential, representative workload does not establish worst-case interrupt stress; normal debug overhead is accepted. Halts do not freeze all peripherals, and public RF idle does not prove internal scheduler quiescence. No watchdog configuration is changed.

@todo: add packet reception workloads when a second device is available; this single-device test does not validate RF packet reception.
