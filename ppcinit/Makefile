# PPCInit is a LoadSeg()ed OpenPCI init tool: no startup code or C library,
# and the entry stub must stay first. It needs only the public OpenPCI headers.
# The test links ClosedPCI's library, to run the tool through the real
# configuration parser and allocator.
CLOSEDPCI ?= ../../ClosedPCI
CROSS ?= m68k-amigaos-
CC = $(CROSS)gcc
VAMOS ?= vamos
CFLAGS = -m68020 -O2 -g -Wall -Wextra -Werror -fno-strict-aliasing
TEST_CPPFLAGS = -I$(CLOSEDPCI)/include -I$(CLOSEDPCI)/sdk/Include -I$(CLOSEDPCI)/build

.PHONY: all check clean FORCE
all: build/Libs/PCI/PPCInit

build/Libs/PCI/PPCInit: ppcinit.c
	mkdir -p $(@D)
	$(CC) -I$(CLOSEDPCI)/sdk/Include -Os -m68020 -Wall -Wextra -Werror -nostartfiles \
	    -nostdlib -fno-toplevel-reorder -s $< -o $@

$(CLOSEDPCI)/build/libclosedpci.a: FORCE
	$(MAKE) -C $(CLOSEDPCI) build/libclosedpci.a

build/test_ppcinit: test_ppcinit.c $(CLOSEDPCI)/build/libclosedpci.a
	mkdir -p $(@D)
	$(CC) $(TEST_CPPFLAGS) $(CFLAGS) $< $(CLOSEDPCI)/build/libclosedpci.a -o $@

check: build/Libs/PCI/PPCInit build/test_ppcinit
	mkdir -p build/vamos
	$(VAMOS) -q -S -s 64 --vols-base-dir $(CURDIR)/build/vamos -C 68020 build/test_ppcinit

clean:
	rm -rf build

FORCE:
