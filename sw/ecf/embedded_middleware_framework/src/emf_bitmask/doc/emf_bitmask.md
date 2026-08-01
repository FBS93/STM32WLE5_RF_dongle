# EMF bitmask overview

EMF bitmask provides a compact bitmask helper with priority-oriented lookup.

# Glossary

| Term | Definition |
|------|------------|
|   |   |

# Usage example

```c
#include "emf_bitmask.h"

static uint8_t bitmask[2U];

void example(void)
{
  EMF_bitmask_clearAll(bitmask, 2U);
  EMF_bitmask_setBit(bitmask, 16U);
  EMF_bitmask_setBit(bitmask, 1U);

  /* Returns 1U because bit position 1 has the highest priority. */
  (void)EMF_bitmask_findMax(bitmask, 2U);
}
```

See also [test_emf_bitmask.md](../test/doc/test_emf_bitmask.md).
