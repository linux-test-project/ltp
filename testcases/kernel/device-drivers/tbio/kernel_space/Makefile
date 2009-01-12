#
# Makefile for GCOV profiling kernel module
#

#KERNELDIR := /usr/src/linux-2.5.64-gcov
EXTRA_CFLAGS := -Wall

ifneq ($(KERNELRELEASE),)

obj-m	:= tbio.o
else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
#	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules
endif

clean:
	rm -f *.o rm -f .*.cmd rm -f *.bb* rm -f *.ko rm -f *.mod.* 2>/dev/null || true

