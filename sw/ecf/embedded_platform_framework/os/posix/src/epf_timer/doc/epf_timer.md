# EPF timer overview

EPF timer provides POSIX thread-based one-shot and periodic timers.

# Glossary

| Term | Definition |
|------|------------|
|   |   |

# Usage example

```c
#include "epf_timer.h"

static EPF_timer_t timer;
static EPF_timer_entry_t tick;

static void onTick(void)
{
}

void example(void)
{
  EPF_timer_init(&timer, 10U);
  EPF_timeEvent_new(&tick, onTick);
  EPF_timer_arm(&timer,
                &tick,
                EPF_TIMER_MS_TO_NS(100U),
                EPF_TIMER_MS_TO_NS(100U));
}
```
