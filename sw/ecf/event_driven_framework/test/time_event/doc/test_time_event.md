# Time event test overview
This test verifies the correct operation and delivery of time events. Its purpose is to confirm that all types of time events fire at the correct rates, that expiration counters behave deterministically and that the publish–subscribe mechanism delivers the events reliably.

# Use case
The test instantiates a single active object (`timeMgr`) that:
- Through its public API, arms and disarms three time events associated with the following signals:
  - `PERIODIC_TIMER_5_MS`
  - `PERIODIC_TIMER_100_MS`
  - `ONE_SHOT_TIMER_100_MS`
- Handles them inside a flat HSM with a single state (`readyState`), where the counters  
  `periodic_5ms_cnt`, `periodic_100ms_cnt` and `one_shot_100ms_cnt` are incremented upon reception.

A high-priority external timer is configured to generate a system tick. This tick is used both to drive the EDF time event subsystem via `EDF_timeEvent_tick()` and to trigger the test verification.

# Verification scope
This test validates:
- Correct arming, disarming and expiration of periodic and one-shot time events.
- Accurate timing behavior, ensuring that each event fires at the expected rate based on the system tick.
- Deterministic updates of the expiration counters exposed by `timeMgr`.
- Reliable delivery of time events through the EDF publish–subscribe mechanism.

This test provides a foundational verification that ensures the timing subsystem behaves deterministically and can be relied upon by any component that depends on time events.
