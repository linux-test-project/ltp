#
# Makefile for GCOV profiling kernel module
#

#KERNELDIR := /usr/src/linux-2.5.66-gcov
EXTRA_CFLAGS := -Wall -Wextra -Wno-unused-parameter

ifneq ($(KERNELRELEASE),)

obj-m	:= nlsTest.o
else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	gcc $(EXTRA_CFLAGS) -o userBlockNLS userBlockNLS.c
#	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules
endif

clean:
	rm -f modules.order
	rm -f Module.symvers
	rm -f userBlockNLS
	rm -f nlsTest.o
	rm -f nlsTest.ko
	rm -f nlsTest.mod.c
	rm -f nlsTest.mod.o
	rm -f .nlsTest.ko.cmd
	rm -f .nlsTest.mod.o.cmd
	rm -f .nlsTest.o.cmd
	rm -rf .tmp_versions
