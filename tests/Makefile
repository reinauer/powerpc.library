AMIGA ?= /opt/amiga-16.2
BUILD ?= ../build/openpci-fix
LIBRARY ?= $(BUILD)/powerpc.library
CC = $(AMIGA)/bin/m68k-amigaos-gcc
VAMOS = $(AMIGA)/bin/vamos

.PHONY: check
check: $(BUILD)/test_bridge_init
	mkdir -p $(BUILD)/vamos
	$(VAMOS) -q -S -s 128 --vols-base-dir $(abspath $(BUILD)/vamos) -C 68040 \
	    $< $(LIBRARY) > $(BUILD)/bridge-init.log 2>&1 || \
	    { cat $(BUILD)/bridge-init.log; exit 1; }
	cat $(BUILD)/bridge-init.log

$(BUILD)/test_bridge_init: bridge_init.c
	mkdir -p $(BUILD)
	$(CC) -m68020 -O2 -Wall -Wextra -Werror $< -o $@
