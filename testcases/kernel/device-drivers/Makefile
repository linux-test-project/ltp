#
# Makefile for GCOV profiling kernel module
#

#KERNELDIR := /usr/src/linux-2.5.64-gcov
CFLAGS := $(CFLAGS) -Wall -g

ifneq ($(KERNELRELEASE),)

obj-m	:= LtpAcpiCmds.o
obj-p   := Test
else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	gcc  $(CFLAGS) -o LtpAcpiMain  LtpAcpiMain.c
#	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules
endif

clean:
	rm -f LtpAcpiCmds.o 2>/dev/null || true

