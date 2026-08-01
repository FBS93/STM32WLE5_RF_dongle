# Embedded Assert Framework (EAF) overview

EAF is a lightweight, platform-agnostic runtime assertion framework designed to run on both embedded systems and host environments. Its purpose is to provide:
- A safe mechanism to validate runtime conditions.
- Atomic evaluation of assertions through automatic critical-section management.
- Consistent way to report errors with contextual information.
- Functional assert mechanism even in deeply embedded, safety-critical paths.
- Lightweight alternative to full-blown diagnostic frameworks.

The library implementation delegates platform-specific functionality (e.g., critical section implementation) to the Embedded Base Framework (EBF) library.

# Glossary

| Term | Definition |
|------|------------|
| Assert | Runtime check that reports an error when an expected condition is false. |
| Design by Contract (DBC) | Practice of expressing assumptions and guarantees in code through executable checks such as asserts. |

# Usage example

```c
#include "eaf.h"
#include "ebf.h"

// Define static file name string for error reporting.
EAF_DEFINE_THIS_FILE("example.c");

void func(int a, int b)
{
  // Assertions with automatic critical section protection.
  EAF_ASSERT(a > 0);
  EAF_ASSERT_ID(b != 0, 101);

  /**
   * Multiple assertions evaluated atomically with a
   * single automatic critical section protection.
   */
  EAF_ASSERT_BLOCK_BEGIN();
      EAF_ASSERT_IN_BLOCK(a > 0);
      EAF_ASSERT_IN_BLOCK_ID(b != 0, 202);
  EAF_ASSERT_BLOCK_END();

  // Error reporting with automatic critical section protection.
  if (a > 100) {
    EAF_ERROR();
  }
  if (b > 100) {
    EAF_ERROR_ID(303);
  }

  EBF_CRITICAL_SECTION_ENTRY();

  /**
   * Assertions intended to be used only inside a critical section
   * (i.e., they do not perform critical section management).
   */
  EAF_ASSERT_IN_CRITICAL_SECTION(a > 0);
  EAF_ASSERT_IN_CRITICAL_SECTION_ID(b != 0, 404);

  /**
   * Error reporting intended only inside a critical section
   * (i.e., they do not perform critical section management).
   */
  if (a > 100) {
    EAF_ERROR_IN_CRITICAL_SECTION();
  }
  if (b > 100) {
    EAF_ERROR_IN_CRITICAL_SECTION_ID(505);
  }

  EBF_CRITICAL_SECTION_EXIT();
}
```
