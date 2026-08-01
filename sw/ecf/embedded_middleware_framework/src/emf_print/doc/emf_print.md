# EMF print overview

EMF print provides minimal formatted output on top of the EBF stdout abstraction.

# Glossary

| Term | Definition |
|------|------------|
|   |   |

# Usage example

```c
#include "emf_print.h"

void example(void)
{
  EMF_PRINT("id=%u temp=%f\n",
            EMF_PRINT_ARG_U(7U),
            EMF_PRINT_ARG_F(23.5f));
}
```
