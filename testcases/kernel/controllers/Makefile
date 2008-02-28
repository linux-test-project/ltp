SUBDIRS = cpuctl
CHECK_CPUCTL = $(shell grep -w cpu /proc/cgroups|cut -f1)
all:
	@set -e;
ifeq ($(CHECK_CPUCTL),cpu)

	for i in $(SUBDIRS); do $(MAKE) -C $$i $@ ;done;
else
	echo "Kernel is not compiled with cpu controller support";
endif

install:
	@set -e; \
        ln -f test_controllers.sh ../../bin/test_controllers.sh;

ifeq ($(CHECK_CPUCTL),cpu)

	for i in $(SUBDIRS); do $(MAKE) -C $$i install ; done; \
	chmod ugo+x test_controllers.sh;
else
	echo "Kernel is not compiled with cpu controller support";
endif

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done
