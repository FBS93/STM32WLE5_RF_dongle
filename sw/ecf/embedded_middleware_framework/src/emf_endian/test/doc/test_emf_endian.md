# Endian test overview
This test verifies the functional behavior exposed by the public API of `emf_endian`.

# Use case
Numeric values are serialized and deserialized across big-endian and little-endian byte orders.

# Verification scope
The test validates externally observable behavior through the public API.
It covers endian read/write functions for round-trip consistency and expected byte layout.
