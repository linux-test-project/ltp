KDIR:=/lib/modules/$(shell uname -r)/build

obj-m := crasher.o

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(RM) .*.cmd *.o *.ko -r .tmp* *.mod.c Modules.symvers
