#
# Include this file in your Makefile for linking against the built NPTL 
# and GLIBC
# 
# NOTE: Before making the test suite, you need to build glibc. $LIBC_PATCH 
# stands for the path of GLIBC, please export it first.

GLIBCROOT :=$(LIBC_PATH)/libc
GLIBCDIR := $(LIBC_PATH)/libc/build

CC := /usr/lib/gcc-snapshot/bin/gcc
CCINCLUDE += /usr/lib/gcc-snapshot/lib/gcc-lib/i386-linux/3.3/include

GLIBC_CFLAGS := \
	-nostdinc \
        -I$(CCINCLUDE) \
        -I$(GLIBCROOT)/root/include{,/asm,/linux} \
        -I$(GLIBCROOT)/root/include \

GLIBC_LDFLAGS := 	$(GLIBCDIR)/nptl/libpthread.so.0 \
        $(GLIBCDIR)/libc.so.6 \
        -Wl,-rpath,$(GLIBCDIR):$(GLIBCDIR)/nptl:$(GLIBCDIR)/elf,-dynamic-linker,$(GLIBCDIR)/elf/ld-linux.so.2

CFLAGS += $(GLIBC_CFLAGS)
LDFLAGS += $(GLIBC_LDFLAGS)
