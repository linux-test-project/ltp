KDIR := /lib/modules/$(shell uname -r)/build

obj-m := lkdtm.o

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(RM) .*.cmd *.o *.ko -r .tmp* *.mod.c .lkdtm* Module*.symvers
