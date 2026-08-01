# Fuzz example overview

Tiny dummy application used to validate the host fuzzing port end-to-end. It provides a minimal HSM (Hierarchical State Machine), a small set of immutable and mutable events, reserved crash/hang paths and a [corpus](../../../../../fuzz/fuzz_example/corpus/) suitable for exercising the template fuzzing workflow. The implemented HSM is the following:

```mermaid
stateDiagram-v2
    [*] --> operational

    state operational {
        [*] --> standby

        standby --> armed: START / reset collection state
        standby --> standby: FINISH / ignored_count++
        standby --> standby: STEP / ignored_count++
        standby --> standby: SET / applySet()

        armed --> armed: START / ignored_count++
        armed --> armed: FINISH / ignored_count++
        armed --> armed: SET / applySet()
        armed --> armed: STEP / rejected slot or adjusted value < gate, ignored_count++
        armed --> collecting: STEP / accepted slot and adjusted value >= gate, beginCollecting(), transition_count++

        collecting --> collecting: SET / applySet()
        collecting --> collecting: STEP / rejected slot, ignored_count++
        collecting --> collecting: STEP / accepted slot and STOP flag clear
        collecting --> standby: STEP / accepted slot and STOP flag set, transition_count++
        collecting --> standby: FINISH / transition_count++
    }

    operational --> operational: PING / ping_count++
    operational --> standby: RESET / resetRuntime(), transition_count++
```

The purpose of this example is to teach the fuzzing flow and stateful discovery, not to model a real application domain.

# Glossary

| Term | Definition |
|---|---|
| HSM | Hierarchical State Machine implemented by an active object. |
| Corpus | Versioned initial input set used to bootstrap the fuzzing campaign. |

# Usage example

This example is built only under the following CMake configuration:
- `EDF_EXAMPLE_FUZZ=ON`
- `EBF_CORE=EBF_CORE_OS`
- `EBF_PORT=EBF_PORT_POSIX_NON_PREEMPTIVE_FUZZ`.
