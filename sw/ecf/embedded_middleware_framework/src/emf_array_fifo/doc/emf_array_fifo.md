# EMF array FIFO overview

EMF array FIFO provides a circular FIFO of fixed-size slots. It is intended for records or frames where every element has the same size.

This module does not perform locking, protect it externally if it is shared by concurrent contexts.

# Glossary

| Term | Definition |
|------|------------|
|   |   |

# Usage example

```c
#include "emf_array_fifo.h"
#include "emf_utils.h"

typedef struct
{
  uint8_t id;
  uint8_t value;
} sample_t;

static EMF_arrayFifo_handler_t fifo;
static EMF_UTILS_MEM_ALIGNED_SLOT(sizeof(sample_t)) fifo_storage[4U];

void example(void)
{
  sample_t in = {1U, 2U};
  sample_t out;

  EMF_arrayFifo_init(&fifo, 4U, sizeof(sample_t), (uint8_t*)fifo_storage);
  EMF_arrayFifo_push(&fifo, (const uint8_t*)&in);
  EMF_arrayFifo_pop(&fifo, (uint8_t*)&out);
}
```

See also [test_emf_array_fifo.md](../test/doc/test_emf_array_fifo.md).
