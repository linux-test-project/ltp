#
# Makefile for GCOV profiling kernel module
#

#KERNELDIR := /usr/src/linux-2.5.64-gcov
EXTRA_CFLAGS := -Wall -W -Wno-unused-parameter

ifneq ($(KERNELRELEASE),)

obj-m	:= tusb.o
else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
#	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -f modules.order

help:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) help

modules_install:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules_install

endif
