#
# Makefile for GCOV profiling kernel module
#

#KERNELDIR := /usr/src/linux-2.5.66-gcov
CFLAGS := $(CFLAGS) -Wall

ifneq ($(KERNELRELEASE),)

obj-m	:= nlsTest.o
else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	gcc -Wall -o userBlockNLS userBlockNLS.c
#	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules
endif

clean:
	rm -f nlsTest.o 2>/dev/null || true

