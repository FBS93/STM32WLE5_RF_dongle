# CRC test overview
This test verifies the functional behavior exposed by the public API of `emf_crc`.

# Use case
Payload integrity is checked by computing CRC-16/CCITT values over byte buffers.

# Verification scope
The test validates externally observable behavior through the public API.
It covers known reference vector results and sensitivity to payload changes.
