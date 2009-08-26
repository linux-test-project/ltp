ifdef CROSS_COMPILE
CHECK_CGROUP := $(shell test -f $(TARGET_DIR)/usr/include/linux/cgroupstats.h && echo 'cgroup')
else
CHECK_CGROUP := $(shell test -f /proc/cgroups && echo 'cgroup')
CHECK_CPUCTL := $(shell grep -w cpu /proc/cgroups 2>/dev/null|cut -f1)
CHECK_MEMCTL := $(shell grep -w memory /proc/cgroups 2>/dev/null|cut -f1)
CHECK_BLOCKIOCTL := $(shell grep -w blockio /proc/cgroups 2>/dev/null|cut -f1)
CHECK_FREEZER := $(shell grep -w freezer /proc/cgroups 2>/dev/null| cut -f1)
CHECK_CPUSETCTL = $(shell grep -w cpuset /proc/cgroups 2>/dev/null|cut -f1)
CHECK_CPUACCTCTL := $(shell grep -w cpuacct /proc/cgroups 2>/dev/null|cut -f1)
endif

ifdef CROSS_COMPILE
ifeq ($(CHECK_CGROUP),cgroup)
SUBDIRS += cgroup
SUBDIRS += cgroup_fj
SUBDIRS += cpuctl
SUBDIRS += cpuctl_fj
SUBDIRS += memctl
SUBDIRS += io-throttle
SUBDIRS += freezer
SUBDIRS += cpuset
SUBDIRS += cpuacct
else
$(info "Kernel is not compiled with control cgroup support")
endif
else
ifeq ($(CHECK_CGROUP),cgroup)
SUBDIRS += cgroup
SUBDIRS += cgroup_fj
else
$(info "Kernel is not compiled with control cgroup support")
endif

ifeq ($(CHECK_MEMCTL),memory)
SUBDIRS += memcg
else
$(info "Kernel is not compiled with memory resource controller support")
endif

ifeq ($(CHECK_CPUCTL),cpu)
SUBDIRS += cpuctl
SUBDIRS += cpuctl_fj
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
ifeq ($(CHECK_CPUSETCTL),cpuset)
SUBDIRS += cpuset
else
$(info "Kernel is not compiled with cpuset resource controller support")
endif
ifeq ($(CHECK_CPUACCTCTL),cpuacct)
SUBDIRS += cpuacct
else
$(info "Kernel is not compiled with cpuacct resource controller support")
endif
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
