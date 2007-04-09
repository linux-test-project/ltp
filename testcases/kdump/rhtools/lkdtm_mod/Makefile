obj-m := lkdtm.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	rm -f *.mod.c *.ko *.o *.cmd
