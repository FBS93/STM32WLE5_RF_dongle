# Embedded target remote Hardware-in-the-Loop (HiL) testing

## Overview

This document describes how automated HiL tests can be executed on the embedded target using a Python runner script integrated with CTest.

The script orchestrates the full remote test workflow through a Raspberry Pi connected to the target device. It requires the [embedded target remote debugging](embedded_target_remote_debugging.md) and [embedded target remote logging](embedded_target_remote_logging.md) to be enabled in order to flash the test firmware, execute it on the target, capture serial output and report PASS/FAIL back to CTest.

## Dependencies

### Devcontainer

Make sure that the environment variables in [devcontainer.json](../../../../.devcontainer/devcontainer.json) are configured for the HiL setup and target environment:

- `TARGET_RX_TIMEOUT_S` indicates the maximum allowed inactivity time (in seconds) during which no logging data is received from the target. If this timeout expires, the HiL test is considered failed.
- `NETWORK_LATENCY_TIMEOUT_S` defines the maximum wait time used by readiness checks for Raspberry Pi services.
- `WORKSPACE_FOLDER` indicates the absolute path to the workspace folder inside the container.

## CTest integration example

The [run_target_test tool](../../../../tools/run_target_test/run_target_test.md) can be used with the CMake `add_test()` function as follows, where `hil_target_test` should be replaced with the CMake target name of your HiL test executable:

```cmake
add_test(
  NAME hil_target_test
  COMMAND
    python3
    ${PROJECT_SOURCE_DIR}/tools/run_target_test/run_target_test.py
    $<TARGET_FILE:hil_target_test>
)
```

When HiL tests are registered this way, they can be executed and reported like regular SiL tests with CTest. They can be run from the command line using CTest or directly from the Testing view in the Activity Bar.

![Testing view in the Activity Bar](../../../assets/testing_view_in_the_activity_bar.png)
