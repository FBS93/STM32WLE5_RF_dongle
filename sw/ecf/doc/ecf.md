# Embedded C Framework (ECF) overview

ECF is a collection of open-source frameworks, adapted and extended to provide a complete embedded software development framework, suitable for both embedded systems and host environments. Each framework can be used independently without restrictions or external dependencies. However, the full potential of ECF is achieved when its frameworks are combined as an integrated whole.

ECF is composed of the following frameworks:
- [Embedded Base Framework (EBF)](../embedded_base_framework/doc/ebf.md)
- [Embedded Assert Framework (EAF)](../embedded_assert_framework/doc/eaf.md)
- [Event Driven Framework (EDF)](../event_driven_framework/doc/edf.md), together with [EDF Test](../event_driven_framework/edf_test/doc/edf_test.md)
- [Embedded Middleware Framework (EMF)](../embedded_middleware_framework/doc/emf.md)
- [Embedded Test Framework (ETF)](../embedded_test_framework/doc/etf.md)
- [Embedded Fake Framework (EFF)](../embedded_fake_framework/doc/eff.md)
- [Embedded Platform Framework (EPF)](../embedded_platform_framework/doc/epf.md)

The following diagram illustrates the ECF structure and the platform dependencies of each framework. See the [Glossary](#glossary) for definitions of the terms used in the diagram.

![ECF Platform Dependencies Diagram](assets/ECF_platform_dependencies_diagram.drawio.png)

ECF is designed according to the following principles:
- No dynamic memory allocation is used.
- Fully deterministic execution with predictable resource management.
- Strict separation between platform-independent and platform-specific code.
- High configurability through [CMake parametrization](../../../CMakeLists.txt), making it suitable for both high-performance and highly resource-constrained platforms.
- Unified stdin/stdout abstraction for consistent and extensible logging across all supported platforms.
- Designed for straightforward testing of the framework and applications built on top of it.
- Designed for dual-target execution model.

## Build configuration

The ECF build configuration is mainly controlled through the following CMake cache variables:

| Variable | Description |
|---|---|
| `EBF_CORE` | Selects the [Core](../embedded_base_framework/doc/ebf.md#glossary) used by EBF. |
| `EBF_PORT` | Selects the [Port](../embedded_base_framework/doc/ebf.md#glossary) used by EBF. |
| `ECF_TARGET_PLATFORM` | Selects the target platform package. When defined, it loads the platform-specific CMake configuration and platform integration files. |
| `ECF_TEST` | Enables `CTest`, the ECF CMake test and mock helper functions and the ECF tests. |

These variables are typically defined in the project [CMakePresets.json](../../../CMakePresets.json).

### Platform integration

When `ECF_TARGET_PLATFORM` is defined, ECF loads the selected platform package and its platform-specific CMake integration.

The `platform.cmake` file of the selected platform package centralizes the common platform-specific build configuration that shall be applied to the relevant embedded executables, such as startup files, linker scripts, libraries and target-specific compiler or linker options.

The [STM32F103C8Tx platform package](../platform/stm32f103c8tx/doc/stm32f103c8tx_platform_package.md) shall be used as the reference example.

### Test and mock integration

When `ECF_TEST` is enabled, ECF loads its CMake functions for test registration and mock integration.

For software tests and mocks using the ECF test and mock utilities, ECF provides the CMake functions defined in [`ecf_test.cmake`](../tools/cmake/functions/ecf_test.cmake) and [`ecf_mock.cmake`](../tools/cmake/functions/ecf_mock.cmake). The function contracts and expected usage are defined in their header documentation.

# Glossary

| Term | Definition |
|------|------------|
| Platform | Combination of execution environment defining the runtime model (bare-metal, RTOS, OS, etc.), hardware architecture and build configuration. Platform selection is performed through the [Embedded Base Framework (EBF)](../embedded_base_framework/doc/ebf.md) configurations. |
| Platform package | Target-specific package bundling the platform implementation and configuration files needed for a concrete platform. |
| Platform-agnostic | Platform-independent, providing identical public APIs and behavior across all supported platforms. |
| Multi-platform | Platform-dependent but with unified public APIs enabling full portability across all supported platforms. |
| Platform-dependent | Platform-specific, providing optimized but non-portable public APIs. |
| Dual-target | Capability to execute on both embedded targets and host environments. |

# Usage example

## Framework examples

For framework-specific usage examples, see the documentation of the frameworks listed in the overview section.

## Platform package example

A concrete embedded target integration is typically grouped as one platform package.

The repository includes the [STM32F103C8Tx platform package](../platform/stm32f103c8tx/doc/stm32f103c8tx_platform_package.md) as the reference example. It keeps that target-specific implementation in a single folder and is enabled at build time as follows:

```cmake
set(ECF_TARGET_PLATFORM stm32f103c8tx)
```

This platform package is used as the reference target for the framework target tests.
