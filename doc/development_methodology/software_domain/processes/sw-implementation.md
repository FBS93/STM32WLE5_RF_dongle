# sw-implementation

## Purpose

Implement the software from software requirements, software architecture and software detailed design.

## Input work products

- Software requirements
- Software architecture
- Software detailed design
- Tools

## Output work products

- Software implementation

## Steps

1. Review the software requirements, software architecture and software detailed design.
2. Review available tools required for the implementation.
3. Implement each software unit according to its software detailed design.
4. Ensure consistency between implementation, software detailed design, software architecture and software requirements.
5. Check the software implementation for completeness, consistency and correctness.

## Guidelines

### Software implementation work product

Each software unit shall be implemented strictly according to its software detailed design and shall not introduce additional behavior.

Software architecture and software requirements shall be used only as complementary input when the software detailed design does not fully define the required behavior.

The implementation of each software unit shall:
- Be written in C.
- Use a build system based on CMake.
- Define a `CMakeLists.txt` file in its root folder.
- Be implemented as an independent static library (`.a`).
- Link dependencies using `PRIVATE` visibility whenever possible.
- Use and link only the external dependencies defined in the software detailed design.
- Expose all header files as `PUBLIC` to allow full validation of the software detailed design. The `<sw_unit_name>.h` header shall be the primary interface intended for usage by other software units.
- Use a library name matching the software unit name defined in the detailed design, using lower_snake_case.
- Follow the rules defined in [embedded_c_guidelines.md](../resources/embedded_c_coding_guidelines.md)
- Implement a source file (`.c`) for each header file (`.h`) defined in the software unit detailed design, using the same name as the corresponding header file.

The [Embedded C Framework (ECF)](../../../../sw/ecf/doc/ecf.md) shall be used as a reference for the implementation of the software units.

#### Active object implementation

The implementation of software units defined as active objects shall:
- Be implemented using [Event Driven Framework (EDF)](../../../../sw/ecf/event_driven_framework/doc/edf.md).
- Implement the Hierarchical State Machine (HSM) exactly as defined in the software detailed design.
  - Implement each state defined in the HSM as a static function in the source file (`<sw_unit_name>.c`), using the names defined in the software detailed design.
  - Implement each activity defined in the HSM in a source file named `<sw_unit_name>_activities.c`, with corresponding declarations in the `<sw_unit_name>_activities.h` header file.
  - Ensure that the HSM only extracts event parameters, evaluates minimal dispatching logic and invokes the corresponding activities.
  - Delegate all functional logic to activity functions and avoid implementing it directly within the HSM.
- Use events as the only mechanism for interaction between software units. Direct function calls to other software units shall not be used.
- Publish and subscribe only to the events explicitly defined for the software unit.
- Include the header-only software unit defining the complete set of events from the corresponding software design.
- Not introduce additional states, activities, transitions, published events, or subscribed events not defined in the software detailed design.

#### Platform package implementation

If the software detailed design specifies a platform package, its implementation shall be reused from [Embedded C Framework (ECF)](../../../../sw/ecf/doc/ecf.md) when available. If not available, it shall be implemented within ECF based on the corresponding platform package software detailed design and upstream elements.

The implementation structure of the platform package software unit deviates from the standard software unit folder structure. The platform package root folder shall not define `inc/`, `src/`, or `u_test/` folders and shall contain:
- `sw/ECF/src/platform/<platform_name>/doc/` containing the software detailed design Markdown document.
- `sw/ECF/src/platform/<platform_name>/cmake/` containing the platform-specific CMake configuration files.
- `sw/ECF/src/platform/<platform_name>/linker/` containing the linker script files.
- `sw/ECF/src/platform/<platform_name>/startup/` containing the startup source files.
- `sw/ECF/src/platform/<platform_name>/svd/` containing the SVD file, which shall always be provided by upstream elements and shall never be implemented from scratch.
- `sw/ECF/src/platform/<platform_name>/stdio/` containing the `stdio` software unit implementation providing [Embedded Base Framework (EBF)](../../../../sw/ecf/embedded_base_framework/doc/ebf.md) stdin/stdout integration as specified in the platform software design and software detailed design. The `stdio` software unit implementation deviates from the standard software unit structure and shall:
  - Contain `sw/ECF/src/platform/<platform_name>/stdio/src/` with the source file implementation providing strong implementations of the weak functions defined in [Embedded Base Framework (EBF)](../../../../sw/ecf/embedded_base_framework/doc/ebf.md).
  - Follow the integration strategy defined in the platform software design.
  - Not define `inc/` or `u_test/` folders.

The platform package shall be integrated into the build system using the mechanisms provided by [Embedded C Framework (ECF)](../../../../sw/ecf/doc/ecf.md), ensuring that all platform-specific elements are applied to all relevant embedded target executables.

The [STM32F103C8Tx platform package](../../../../sw/ecf/platform/stm32f103c8tx/doc/stm32f103c8tx_platform_package.md) shall be used as a reference for platform package implementation.

##### Toolchains implementation

The toolchains defined in the platform package software detailed design or its upstream elements shall be implemented using CMake toolchain files in the `sw/src/tools/cmake/toolchains` folder.

The [Embedded C Framework (ECF) toolchains](../../../../tools/cmake/toolchains) shall be used as a reference for toolchain implementation, reusing existing definitions whenever available.

##### Build presets implementation

The build presets defined in the platform package software detailed design or its upstream elements shall be implemented in the `sw/src/CMakePresets.json` file.

The [Embedded C Framework (ECF) `CMakePresets.json`](../../../../CMakePresets.json) shall be used as a reference for build presets implementation, reusing existing definitions whenever available.

##### Debug configuration implementation

The debug configuration defined in the platform package software detailed design or its upstream elements shall be implemented in the `sw/src/.vscode/launch.json` file.

The [Embedded C Framework (ECF) launch.json](../../../../.vscode/launch.json) shall be used as a reference for debug configuration implementation, reusing existing definitions whenever available.
