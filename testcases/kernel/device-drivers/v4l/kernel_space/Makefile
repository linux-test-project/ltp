
EXTRA_CFLAGS += -Wall -W -Wno-unused-parameter

ifneq ($(KERNELRELEASE),)

obj-m	:= video_dummy.o
else
KDIR	:= /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

modules:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -f modules.order

help:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) help

modules_install:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules_install

endif
