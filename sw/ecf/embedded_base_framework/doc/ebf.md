# Embedded Base Framework (EBF) overview

EBF is a lightweight, multi-platform base framework designed to run on both embedded systems and host environments. Its purpose is to provide:
- A compact and portable base layer across all platforms supported by ECF.
- Unified critical-section management, abstracted behind macros.
- Extensible stdin/stdout interfaces using weak functions.
- Utility functions with hardware-accelerated implementation depending on the selected platform.

When implementing stdin/stdout interfaces by overriding EBF weak functions, be aware that the resulting I/O behavior is implementation-dependent and may be blocking. Serializing a byte stream without adequate buffering or flow control can cause libraries using stdout to block until all bytes are transmitted. Avoiding TX blocking requires a properly sized transmit buffer and asynchronous transmission. Likewise, RX handling must be designed according to the use case to prevent data loss.

If stdin/stdout can be used from multiple contexts with different priorities (i.e. they act as shared resources), their implementation shall be protected using `EBF_CRITICAL_SECTION_ENTRY()` and `EBF_CRITICAL_SECTION_EXIT()`.

All configuration options for this library are documented in the "EBF default configuration" section of [CMakeLists.txt](../../../../CMakeLists.txt).

# Glossary

| Term | Definition |
|------|------------|
| Core | Base execution environment defining the runtime model (e.g., bare-metal, RTOS, OS). |
| Port | Core-specific adaptation to a concrete hardware and/or build configuration (e.g., ARM for the bare-metal core, POSIX for the OS core). |

# Usage example

```c
#include "ebf.h"

/**
 * Custom stdin handler.
 */
static void myStdinHandler(uint8_t const *data, uint16_t len)
{
    // Process incoming bytes ...
}

/**
 * Custom stdin listener registration.
 */
void EBF_setStdinListener(EBF_stdin_t const listener)
{
    // Store listener pointer ...
}

/**
 * Custom stdout writer.
 * Can output to any platform-specific channel
 * (e.g., UART/SPI/... on target, console on host).
 */
void EBF_stdoutWrite(const uint8_t *data, uint16_t const len)
{
    // Process outgoing bytes ...
}

/**
 * Custom stdout writer check.
 */
bool EBF_stdoutIsReadyToWrite(uint16_t const len) {
    // Check if stdout can send 'len' bytes ...
}

void example(void)
{
    EBF_setStdinListener(myStdinHandler);

    EBF_CRITICAL_SECTION_ENTRY();

    // Write to stdout if ready
    if (EBF_stdoutIsReadyToWrite(4)) {
        uint32_t value = 0xAABBCCDD;
        EBF_stdoutWrite(&value, sizeof(value));
    }

    EBF_CRITICAL_SECTION_EXIT();

    while(1) {
        EBF_NOP();
    }
}

```
