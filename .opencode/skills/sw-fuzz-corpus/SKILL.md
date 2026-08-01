---
name: sw-fuzz-corpus
description: >
  Analyze EDF fuzzable events to generate an AFL++ fuzz corpus.
  Trigger: When preparing fuzz corpus or improving EDF fuzz coverage.
---

## When to Use

- When an EDF application already defines `EDF_FUZZ_EVENT_TABLE(...)`
- When preparing the initial AFL++ corpus for a fuzz campaign
- When improving low fuzz coverage by refining the corpus

## Critical Patterns

- Analyze only the source code that actually reaches the final fuzzed executable selected for the active CMake build target.
- Read the fuzzable event registry from `EDF_FUZZ_EVENT_TABLE(...)`; do not invent additional fuzzable events.
- Consider only fuzzable events that are actually subscribed by the application active objects.
- Reconstruct the event flow to generate meaningful corpus that exercises important branches and state transitions.
- Propose corpus as raw stdin inputs aligned with the [EDF fuzzing port contract](../../../sw/ecf/event_driven_framework/doc/edf.md#fuzzing-port-contract) format.
- Store the generated AFL++ fuzz corpus in `sw/fuzz/<target>/corpus/`.
- Prefer a small, meaningful initial corpus over a larger random corpus.

## Suggested Workflow

1. Locate `EDF_FUZZ_EVENT_TABLE(...)` of the active CMake build target.
2. Enumerate its registered immutable and mutable events.
3. Check which of those events are actually subscribed and relevant for the application.
4. Trace where those events are consumed and what state transitions or branches they trigger.
5. Generate the corpus seeds as raw byte inputs that trigger those flows, aligned with the EDF fuzzing port contract.
6. Store the generated AFL++ fuzz corpus in `sw/fuzz/<target>/corpus/`.

## Resources

- [EDF](../../../sw/ecf/event_driven_framework/doc/edf.md)
- [EDF fuzzing port contract](../../../sw/ecf/event_driven_framework/doc/edf.md#fuzzing-port-contract)
- [Fuzz example](../../../sw/ecf/event_driven_framework/examples/fuzz_example/doc/fuzz_example.md)
- [Corpus example](../../../sw/fuzz/fuzz_example/corpus/)
