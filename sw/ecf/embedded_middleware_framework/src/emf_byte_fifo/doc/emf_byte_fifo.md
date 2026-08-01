# EMF byte FIFO overview

EMF byte FIFO provides a circular FIFO for byte streams.

This module does not perform locking, protect it externally if it is shared by concurrent contexts.

# Glossary

| Term | Definition |
|------|------------|
|   |   |

# Usage example

```c
#include "emf_byte_fifo.h"

static EMF_byteFifo_handler_t fifo;
static uint8_t storage[16U];

void example(void)
{
  uint8_t data[3U] = {0x11U, 0x22U, 0x33U};
  uint8_t out[2U];

  EMF_byteFifo_init(&fifo, sizeof(storage), storage);
  EMF_byteFifo_push(&fifo, data, 3U);
  EMF_byteFifo_peek(&fifo, out, 2U);
  EMF_byteFifo_drop(&fifo, 2U);
}
```

See also [test_emf_byte_fifo.md](../test/doc/test_emf_byte_fifo.md).
