# EMF utils overview

EMF utils groups helper macros and functions shared across the frameworks.

# Glossary

| Term | Definition |
|------|------------|
|   |   |

# Usage example

```c
#include "emf_utils.h"

typedef struct
{
  uint32_t id;
  uint16_t value;
} sample_t;

EMF_UTILS_MEM_ALIGNED_SLOT(sizeof(sample_t)) storage[4U];

void example(void)
{
  uint8_t buff[8U];

  (void)storage;
  EMF_utils_clear(buff, sizeof(buff));
}
```

See also [test_emf_utils.md](../test/doc/test_emf_utils.md).
