# Embedded Base Framework (EBF) overview

EBF is a lightweight, multi-platform base framework designed to run on both embedded systems and host environments. Its purpose is to provide:
- A compact and portable base layer across all platforms supported by ECF.
- Unified critical-section management, abstracted behind macros.
- Extensible stdin/stdout interfaces using weak functions.
- Utility functions with hardware-accelerated implementation depending on the selected platform.

When implementing stdin/stdout interfaces by overriding EBF weak functions, be aware that the resulting I/O behavior is implementation-dependent and may be blocking. Serializing a byte stream without adequate buffering or flow control can cause libraries using stdout to block until all bytes are transmitted. Avoiding TX blocking requires a properly sized transmit buffer and asynchronous transmission. Likewise, RX handling must be designed according to the use case to prevent data loss.

Stdin/stdout can be used from multiple contexts with different priorities (i.e. they act as shared resources). Therefore, all its implementations shall be protected using `EBF_CRITICAL_SECTION_ENTRY()` and `EBF_CRITICAL_SECTION_EXIT()`.

All configuration options for this library are documented in the "EBF default configuration" section of [CMakeLists.txt](../../../../CMakeLists.txt).

# Glossary

| Term | Definition |
|------|------------|
| Core | Base execution environment defining the runtime model (e.g., bare-metal, RTOS, OS). |
| Port | Core-specific adaptation to a concrete hardware and/or build configuration (e.g., ARM for the bare-metal core, POSIX for the OS core). |

# Usage example

```c
#include "ebf.h"
#include "eaf.h"

/**
 * Custom stdin handler.
 */
static void myStdinHandler(const uint8_t *data, uint16_t len)
{
  // Process incoming bytes ...
}

/**
 * Custom stdin listener registration.
 */
void EBF_setStdinListener(EBF_stdin_t listener)
{
  EBF_CRITICAL_SECTION_ENTRY();

  // Store listener pointer ...

  EBF_CRITICAL_SECTION_EXIT();
}

/**
 * Custom stdout writer.
 * Can output to any platform-specific channel
 * (e.g., UART/SPI/... on target, console on host).
 */
bool EBF_stdoutWrite(const uint8_t *data, uint16_t len)
{
  EBF_CRITICAL_SECTION_ENTRY();

  // Process all outgoing bytes and return true or return false.

  EBF_CRITICAL_SECTION_EXIT();

  return ...;
}

void example(void)
{
  uint32_t value;

  value = 0xAABBCCDDU;

  EBF_setStdinListener(myStdinHandler);

  EAF_ASSERT(EBF_stdoutWrite((const uint8_t *)&value, sizeof(value)));

  while(1) {
    EBF_NOP();
  }
}

```
