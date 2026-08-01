# EMF log overview

EMF log is a thin logging layer built on top of [emf_print](../../emf_print/doc/emf_print.md).

# Glossary

| Term | Definition |
|------|------------|
| Log level | Threshold used to filter logging messages. |

# Usage example

```c
#include "emf_log.h"

void example(void)
{
  EMF_log_init();
  EMF_log_setLevel(LOG_LEVEL_DEBUG);
  EMF_LOG_INFO("value=%u\n", EMF_LOG_ARG_U(10U));
}
```
