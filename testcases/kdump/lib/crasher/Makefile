obj-m := crasher.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules


clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f Module.symvers
#clean:
#	rm -f *.mod.c *.ko *.o .*.cmd *.symvers
#	rm -rf .tmp_versions
