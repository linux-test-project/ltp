CHECK_CGROUP := $(shell test -f /proc/cgroups && echo 'cgroup')
CHECK_CPUCTL := $(shell grep -w cpu /proc/cgroups 2>/dev/null|cut -f1)
CHECK_MEMCTL := $(shell grep -w memory /proc/cgroups 2>/dev/null|cut -f1)
CHECK_BLOCKIOCTL := $(shell grep -w blockio /proc/cgroups 2>/dev/null|cut -f1)
CHECK_FREEZER := $(shell grep -w freezer /proc/cgroups 2>/dev/null| cut -f1)

ifeq ($(CHECK_CGROUP),cgroup)
SUBDIRS += cgroup
else
$(info "Kernel is not compiled with control cgroup support")
endif

ifeq ($(CHECK_CPUCTL),cpu)
SUBDIRS += cpuctl
else
$(info "Kernel is not compiled with cpu controller support")
endif
ifeq ($(CHECK_MEMCTL),memory)
SUBDIRS += memctl
else
$(info "Kernel is not compiled with memory resource controller support")
endif
ifeq ($(CHECK_BLOCKIOCTL),blockio)
SUBDIRS += io-throttle
else
$(info "Kernel is not compiled with blockio resource controller support")
endif
ifeq ($(CHECK_FREEZER),freezer)
SUBDIRS += freezer
else
$(info "Kernel is not compiled with cgroup freezer support")
endif

# If at least one of the controllers is available then build libcontrollers.
ifneq ($(SUBDIRS),)
SUBDIRS := libcontrollers $(SUBDIRS)
endif

.PHONY: all install clean

all:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i $@ ; done

install: test_controllers.sh
	@chmod ugo+x test_controllers.sh
	@ln -f test_controllers.sh ../../bin/test_controllers.sh
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i $@ ; done

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i $@ ; done
