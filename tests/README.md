# Library initialization regression test

Build the library with the normal vbcc cross toolchain, then run:

```
make -f Makefile.cross BUILD=build/openpci-fix TOOLCHAIN=/path/to/toolchain
make -C tests check
```

The test loads that actual mixed 68k/PPC hunk file through AmigaDOS and
calls its resident initialization entry under vamos. Exec, expansion,
Intuition and PCI vectors are simulated. The entry receives no vbcc data
base in a4. No PPC code or physical hardware is executed.

Cases cover a physical FireStorm, its OpenPCI resource replacement, hiding
on OpenLibrary, rejection of an original Prometheus or non-bridge resource,
missing or inconsistent PCI host offsets, cards on different host bridges,
native prometheus.library, early initialization failures, and a single
error requester after a Harrier reservation failure. Every failure checks that only
opened libraries were closed and that all such references were balanced.

`AMIGA` selects the GCC/vamos installation (default `/opt/amiga-16.2`).
`BUILD` selects the test output directory; `LIBRARY` selects a different
library to test. Paths are relative to the tests directory. Output goes to
`BUILD/bridge-init.log` because the current vamos runtime requires seekable
stdout for this GCC C runtime.
