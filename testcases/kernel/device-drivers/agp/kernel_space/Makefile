#
# Makefile for GCOV profiling kernel module
#

#KERNELDIR := /usr/src/linux-2.5.64-gcov

CFLAGS := $(CFLAGS) -Wall
ifneq ($(KERNELRELEASE),)

obj-m	:= tagp.o
else

KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR)  SUBDIRS=$(PWD) modules
#	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules
endif

clean:
	rm -f tagp.o 2>/dev/null || true

