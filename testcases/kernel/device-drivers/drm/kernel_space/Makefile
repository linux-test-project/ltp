#
# Makefile for GCOV profiling kernel module
#
CFLAGS := $(CFLAGS) -Wall
ifneq ($(KERNELRELEASE),)
CFLAGS += -Idrivers/char/drm
obj-m	:= tdrm.o

else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD)  modules
endif

clean:
	rm -f tdrm.o 2>/dev/null || true

