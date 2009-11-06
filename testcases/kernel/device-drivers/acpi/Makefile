#
# Makefile for GCOV profiling kernel module
#
#  1 Mar 2009 Márton Németh <nm127@freemail.hu>
#   - Updated for Linux kernel 2.6.28
#

#KERNELDIR := /usr/src/linux-2.5.64-gcov
EXTRA_CFLAGS := -Wall -g -W -Wno-unused-parameter

ifneq ($(KERNELRELEASE),)

obj-m	:= LtpAcpiCmds.o
obj-p   := Test
else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

modules:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	${CC}  $(CFLAGS) -o LtpAcpiMain  LtpAcpiMain.c

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -f modules.order
	rm -f LtpAcpiMain

help:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) help

modules_install:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules_install

endif
