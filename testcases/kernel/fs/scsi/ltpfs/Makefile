#
# Makefile for GCOV profiling kernel module
#

#KERNELDIR := ../linux-2.5.73
CFLAGS := $(CFLAGS) -Wall -g

ifneq ($(KERNELRELEASE),)

obj-m	:= LtpfsCmds.o
obj-p   := ltpfstest
else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	${CC}  $(CFLAGS) -o ltpfstest -lm  main.c
#	$(MAKE) -C $(KERNELDIR) SUBDIRS=$(PWD) modules
endif

clean:
	rm -f LtpfsCmds.o
	rm -f LtpfsCmds.ko
	rm -f LtpfsCmds.bb
	rm -f LtpfsCmds.bbg
	rm -f LtpfsCmds.mod.c
	rm -f LtpfsCmds.mod.o
	rm -f .*.mod*
	rm -f .*.cmd
