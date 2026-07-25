# Source Code

This directory contains all source files for the Unix shell implementation.

## Known Bugs

### `open` does not check for failure

Objective: Catch errors and exit cleanly when opening files

### Pipe environment setup results in infinite loop or no output

Objective: Diagnose the root of the bug using GDB and fix

Potential Causes:
- execv() not given clean arguments/environment setup
- temp file handoffs between linked commands have improper order or incomplete linkage
- STDIN file descriptor not properly resetting to original STDIN
