# COBS test overview
This test verifies the functional behavior exposed by the public API of `emf_cobs`.

# Use case
Binary payloads are encoded and decoded with COBS framing to guarantee delimiter-safe packet boundaries.

# Verification scope
The test validates externally observable behavior through the public API.
It covers macro contracts, encoding boundaries, round-trip integrity, delimiter handling and failure behavior for malformed encoded input.
