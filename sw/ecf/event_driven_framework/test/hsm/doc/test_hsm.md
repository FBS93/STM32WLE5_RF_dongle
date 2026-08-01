# HSM test overview
This test verifies the correct operation of a deeply nested hierarchical state machine (HSM). The HSM includes multiple levels of hierarchy, transitions between siblings, parent/child states, ancestor/descendant paths and a choice pseudo-state. The purpose of this test is to confirm that the HSM engine evaluates transitions, entry/exit sequences and initial transitions strictly according to the HSM rules.

# Use case
The test instantiates one active object (`hsm`) that implements the full HSM defined in the following diagram:

```mermaid
stateDiagram-v2
state "top" as top {
  [*] --> S1 : Initial
  state "S1" as S1 {
    [*] --> S11
    state "S11" as S11 {
      [*] --> S111
      state "S111" as S111 {
        [*] --> S1111
        state "S1111" as S1111 {
          [*] --> S11111
          state "S11111" as S11111
        }
      }
    }
  }
  state "S2" as S2 {
    state "S21" as S21 {
      state "S211" as S211 {
        [*] --> S2111
        state "S2111" as S2111
        state "S2112" as S2112
      }
    }
  }
  state "S3" as S3 {
    state "S31" as S31 {
      state "S311" as S311
    }
  }
  state "S4" as S4 {
    [*] --> S41
    state "S41" as S41
  }
  state G <<choice>>
}

  S1 --> S211
  S2111 --> S2112
  S2112 --> S2
  S2 --> S21
  S21 --> S31
  S31 --> S3
  S3 --> S311
  S311 --> S41
  S41 --> G
  G --> S41 : [self_transition == true]
  G --> S4 : [self_transition == false]
```

Each test case publishes the same signal (`TRANSITION`) and the HSM must reach the next expected state according to the predefined transition flow:
- Initial transition: (S11111)
- S1 --> S211 : Event bubble up + Last common ancestor + Initial transition (S2111)
- S2111 --> S2112 : Sibling (S2112)
- S2112 --> S2 : Descendant to Ancestor (S2)
- S2 --> S21 : Parent to Child (S21)
- S21 --> S31 : Last common ancestor (S31)
- S31 --> S3 : Child to Parent (S3)
- S3 --> S311 : Ancestor to Descendant (S311)
- S311 --> S41 : Last common ancestor (S41)
- S41:
  - --> S41: Self transition (S41) [self_transition == true]
  - --> S4: Child to Parent + Initial transition (S41) [self_transition == false]

# Verification scope
This test validates:
- Correct traversal of the hierarchical state structure, including all kinds of transitions across multiple nesting levels.
- Proper resolution of transitions according to HSM rules.
- Deterministic behavior of the guarded transitions from S41.
- Stable and predictable final state after each `TRANSITION` event, ensuring consistent HSM execution.

This test provides a foundational verification that ensures all functionality builds on a solid and predictable hierarchical state machine model.
