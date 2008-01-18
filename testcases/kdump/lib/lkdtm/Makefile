obj-m := lkdtm.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
USE_SYMBOL_NAME := $(shell echo "${USE_SYMBOL_NAME}")

ifeq (1, $(USE_SYMBOL_NAME))
	EXTRA_CFLAGS := -DUSE_SYMBOL_NAME
endif

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules


clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f Module.symvers
#clean:
#	rm -f *.mod.c *.ko *.o .*.cmd *.symvers
#	rm -rf .tmp_versions
