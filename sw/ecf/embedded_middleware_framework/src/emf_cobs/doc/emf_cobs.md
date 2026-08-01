# EMF COBS overview

EMF COBS provides Consistent Overhead Byte Stuffing encoding and decoding for byte-stream framing.

# Glossary

| Term | Definition |
|------|------------|
| Packet delimiter | Reserved byte value marking the end of one encoded packet. |

# Usage example

```c
#include "emf_cobs.h"

static uint8_t encoded[EMF_COBS_ENCODED_SIZE(4U)];
static uint8_t decoded[4U];

void example(void)
{
  uint8_t raw[4U] = {0x11U, 0x00U, 0x22U, 0x33U};
  uint16_t encoded_len;
  uint16_t decoded_len;

  EMF_cobs_encode(raw, 4U, encoded, &encoded_len);
  (void)EMF_cobs_decode(encoded, encoded_len, decoded, &decoded_len);
}
```

See also [test_emf_cobs.md](../test/doc/test_emf_cobs.md).
