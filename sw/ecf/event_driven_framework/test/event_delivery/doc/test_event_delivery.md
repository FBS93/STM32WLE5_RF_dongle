# Event delivery test overview
This test verifies that the publish–subscribe mechanism correctly routes events only to the active objects subscribed to each signal. Its purpose is to confirm that event delivery behaves deterministically, without relying on additional application logic.

# Use case
This test defines several active objects, each with a specific subscription pattern:
- Every active object except the last subscribes to exactly one signal.
- The last active object subscribes to all signals.

When an event is published, only the active objects subscribed to that signal should mark the event as received.

This models a typical event-driven scenario: Publishers remain unaware of subscribers and the infrastructure alone is responsible for enforcing correct routing.

# Verification scope
This test validates:
- Events are delivered only to subscribed active objects.
- Non-subscribed active objects never react.
- No cross-delivery or unexpected delivery occurs.
- Subscriptions are enforced as the routing contract.
- The publish–subscribe mechanism behaves predictably and without side effects.

This test provides a foundational verification that ensures all functionality builds on a solid and predictable event-delivery model.