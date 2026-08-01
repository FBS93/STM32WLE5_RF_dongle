# EFF test overview

This test validates the runtime behavior of generated EFF mocks using a dummy module header that exercises parser-sensitive C declarations.

# Use case

The fake generator is used on a small API that includes parser-sensitive C declarations.

# Verification scope

The test verifies that the generated mock can be compiled and used preserving key EFF features.
