#
# Makefile for GCOV profiling kernel module
#

EXTRA_CFLAGS := -Wall -W -Wno-unused-parameter

ifneq ($(KERNELRELEASE),)

obj-m	:= nlsTest.o
else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	${CC} $(EXTRA_CFLAGS) -o userBlockNLS userBlockNLS.c
#	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -f modules.order
	rm -f userBlockNLS

endif
