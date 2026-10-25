# CLI development test overview

Regression checks for the [CLI](../cli.md), using standard-library `unittest` without product hardware. These are development checks, not formal software-domain unit tests; see [SW_ARCH_DESIGN_3](../../arch/sw_designs.md#sw_arch_design_3).

# Use case

Verify CLI request encoding, response decoding and RX/CAD operation handling with simulated serial I/O and local POSIX pseudo-terminals.

# Verification scope

Checks cover command parsing and validation, framing and configuration serialization, output records, partial I/O, deadlines, failures and RX/CAD lifecycle handling, including Ctrl-C/STOP.

They do not validate USB hardware, firmware acceptance, RF behavior or system conformance. See the [CLI usage notes](../cli.md#usage-example) for implementation limits.
