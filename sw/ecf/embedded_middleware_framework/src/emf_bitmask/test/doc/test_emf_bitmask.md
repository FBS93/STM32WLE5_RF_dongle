# Bitmask test overview
This test verifies the functional behavior exposed by the public API of `emf_bitmask`.

# Use case
A compact bitmask tracks flags that can be set, cleared, queried and searched by priority.

# Verification scope
The test validates externally observable behavior through the public API.
It covers clearing, querying any/set bits, setting and clearing individual bits and resolving the highest-priority active bit.
