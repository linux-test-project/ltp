SUBDIRS = libcontrollers cpuctl memctl io-throttle
CHECK_CPUCTL = $(shell grep -w cpu /proc/cgroups 2>/dev/null|cut -f1)
CHECK_MEMCTL = $(shell grep -w memory /proc/cgroups 2>/dev/null|cut -f1)
CHECK_BLOCKIOCTL= $(shell grep -w blockio /proc/cgroups 2>/dev/null|cut -f1)
all:
	@set -e;
ifeq ($(CHECK_CPUCTL),cpu)

	for i in $(SUBDIRS); do $(MAKE) -C $$i $@ ;done;
else
	@echo "Kernel is not compiled with cpu controller support";
endif

ifeq ($(CHECK_MEMCTL),memory)

	for i in $(SUBDIRS); do $(MAKE) -C $$i $@ ;done;
else
	@echo "Kernel is not compiled with memory resource controller support";
endif

ifeq ($(CHECK_BLOCKIOCTL),blockio)

	for i in $(SUBDIRS); do $(MAKE) -C $$i $@ ;done;
else
	@echo "Kernel is not compiled with blockio resource controller support";
endif

install:
	@set -e; \
        ln -f test_controllers.sh ../../bin/test_controllers.sh;

ifeq ($(CHECK_CPUCTL),cpu)

	for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done; \
	chmod ugo+x test_controllers.sh;
else
	@echo "Kernel is not compiled with cpu controller support";
endif

ifeq ($(CHECK_MEMCTL),memory)

	for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done; \
	chmod ugo+x test_controllers.sh;
else
	@echo "Kernel is not compiled with memory resource controller support";
endif

ifeq ($(CHECK_BLOCKIOCTL),blockio)

	for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done; \
	chmod ugo+x test_controllers.sh;
else
	@echo "Kernel is not compiled with blockio resource controller support";
endif

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
