# EMF CRC overview

EMF CRC provides CRC calculation utilities.

# Glossary

| Term | Definition |
|------|------------|
|   |   |

# Usage example

```c
#include "emf_crc.h"

void example(void)
{
  static const uint8_t frame[3U] = {0x01U, 0x02U, 0x03U};
  uint16_t crc;

  crc = EMF_crc_16CCITT(frame, 3U);
  (void)crc;
}
```

See also [test_emf_crc.md](../test/doc/test_emf_crc.md).
