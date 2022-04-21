# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 Linux Test Project

top_srcdir		?= ../../..

include $(top_srcdir)/include/mk/testcases.mk

ASFLAGS =
CPPFLAGS += -I$(abs_srcdir)/include
GUEST_CPPFLAGS = $(CPPFLAGS) -DCOMPILE_PAYLOAD
GUEST_CFLAGS = -ffreestanding -O2 -Wall -fno-asynchronous-unwind-tables -mno-mmx -mno-sse
GUEST_LDFLAGS = -nostdlib -Wl,--build-id=none
GUEST_LDLIBS =

FILTER_OUT_MAKE_TARGETS := lib_guest lib_host lib_x86

ifeq ($(HOST_CPU),x86_64)
	ifneq (,$(findstring m32,$(CFLAGS)))
		HOST_CPU = x86
		GUEST_CFLAGS += -m32
		ASFLAGS += --32
	endif
endif

# Some distros enable -pie by default. That breaks KVM payload linking.
ifdef LTP_CFLAGS_NOPIE
	GUEST_CFLAGS += -fno-pie
	GUEST_LDFLAGS += -no-pie
endif

GUEST_LDFLAGS += -Wl,-T$(abs_srcdir)/linker/$(HOST_CPU).lds
ARCH_OBJ = bootstrap_$(HOST_CPU).o

ifeq ($(HOST_CPU),x86_64)
ARCH_OBJ += lib_x86.o
BIN_FORMAT=elf64-x86-64
else ifeq ($(HOST_CPU),x86)
ARCH_OBJ += lib_x86.o
BIN_FORMAT=elf32-i386
else
MAKE_TARGETS =
endif

lib_guest.o $(ARCH_OBJ): CPPFLAGS	:= $(GUEST_CPPFLAGS)
lib_guest.o $(ARCH_OBJ): CFLAGS		:= $(GUEST_CFLAGS)

include $(top_srcdir)/include/mk/generic_leaf_target.mk

%-payload.o: %.c lib_guest.o $(ARCH_OBJ)
ifdef VERBOSE
	$(CC) $(GUEST_CPPFLAGS) $(GUEST_CFLAGS) $(GUEST_LDFLAGS) -o $*-payload.elf $^ $(GUEST_LDLIBS)
	objcopy -O binary -j .init.boot -j .text -j .data -j .init -j .preinit_array -j .init_array --gap-fill=0 $*-payload.elf $*-payload.bin
	ld -r -T $(abs_srcdir)/linker/payload.lds --oformat=$(BIN_FORMAT) -o $@ $*-payload.bin
else
	@$(CC) $(GUEST_CPPFLAGS) $(GUEST_CFLAGS) $(GUEST_LDFLAGS) -o $*-payload.elf $^ $(GUEST_LDLIBS)
	@objcopy -O binary -j .init.boot -j .text -j .data -j .init -j .preinit_array -j .init_array --gap-fill=0 $*-payload.elf $*-payload.bin
	@ld -r -T $(abs_srcdir)/linker/payload.lds --oformat=$(BIN_FORMAT) -o $@ $*-payload.bin
	@echo KVM_CC $(target_rel_dir)$@
endif
	@rm $*-payload.elf $*-payload.bin

$(MAKE_TARGETS): %: %-payload.o lib_host.o
