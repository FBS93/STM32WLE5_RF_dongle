# Embedded Fake Framework (EFF) overview

EFF is a lightweight, platform-agnostic fake functions framework designed to run on both embedded systems and host environments. Its purpose is to provide:
- Macro-based creation of fake functions for testing and validation.
- Fake functions autogeneration tool.

The ECF framework provides [ECF cmake functions](../../tools/cmake/functions/ecf_mock.cmake) that encapsulate the usage of the [EFF autogeneration tool](../../tools/eff_gen/eff_gen.md).

@todo The current generator intentionally targets a pragmatic subset of the C declarations used in this template. If EFF needs full-language coverage in the future, evaluate integrating a dedicated C parser instead of continuing to grow regex-driven parsing. Good candidates include `pycparser` for a pure Python AST workflow, `libclang` via `clang.cindex`, or other maintained clang bindings capable of handling compiler attributes, nested declarators, macros and more complex function pointer forms.

This framework is a derivative work based on:
- fff (c) Meekrosoft.
- fff-mock-gen (c) Amcolex.

# Glossary

| Term | Definition |
|------|------------|
| Fake Function | Lightweight function replacement used to isolate code under test, control external dependencies and monitor interactions. Used synonymously with Mock Function in the context of this library. |
| Mock Function | Lightweight function replacement used to isolate code under test, control external dependencies and monitor interactions. Used synonymously with Fake Function in the context of this library. |

# Usage example

When the same fake shall be shared across translation units, declare it in one header and define it once in one source file:

```c
/* sensor_mocks.h */
#include "eff.h"

DECLARE_FAKE_VALUE_FUNC(int, sensor_read, uint8_t);
DECLARE_FAKE_VOID_FUNC(sensor_write, const uint8_t*, uint16_t);
```

```c
/* sensor_mocks.c */
#include "sensor_mocks.h"

DEFINE_EFF_GLOBALS;
DEFINE_FAKE_VALUE_FUNC(int, sensor_read, uint8_t);
DEFINE_FAKE_VOID_FUNC(sensor_write, const uint8_t*, uint16_t);
```

```c
/* test_sensor.c */
#include "sensor_mocks.h"

void example(void)
{
  int sensor_values[2U] = {10, 20};
  uint8_t command[2U] = {0xA5U, 0x5AU};

  RESET_FAKE(sensor_read);
  RESET_FAKE(sensor_write);
  EFF_RESET_HISTORY();

  SET_RETURN_SEQ(sensor_read, sensor_values, 2U);

  (void)sensor_read(1U);
  (void)sensor_read(1U);
  sensor_write(command, sizeof(command));

  (void)sensor_read_fake.call_count;
  (void)sensor_write_fake.arg1_val;
}
```
