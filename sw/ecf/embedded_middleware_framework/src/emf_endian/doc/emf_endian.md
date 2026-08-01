# EMF endian overview

EMF endian provides buffer-oriented big-endian and little-endian conversion helpers for generic values and selected fixed-width integer types. It is useful when a protocol format shall remain independent from the host CPU endianness.

# Glossary

| Term | Definition |
|------|------------|
| Big-endian | Byte order where the most significant byte is stored first. |
| Little-endian | Byte order where the least significant byte is stored first. |

# Usage example

```c
#include "emf_endian.h"

void example(void)
{
  uint8_t buff[4U];
  uint32_t in = 0x11223344UL;
  uint32_t out;

  EMF_endian_u32WriteBuffBE(buff, &in);
  EMF_endian_u32ReadBuffBE(buff, &out);
}
```

See also [test_emf_endian.md](../test/doc/test_emf_endian.md).
