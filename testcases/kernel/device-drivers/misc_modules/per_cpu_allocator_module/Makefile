
EXTRA_CFLAGS += -Wall -W -Wno-unused-parameter

ifneq ($(KERNELRELEASE),)

obj-m	:= test_per_cpu_allocator_module.o
else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f modules.order

help:
	$(MAKE) -C $(KDIR) M=$(PWD) help

modules_install:
	$(MAKE) -C $(KDIR) M=$(PWD) modules_install

endif
