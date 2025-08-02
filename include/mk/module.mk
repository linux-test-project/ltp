# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) Linux Test Project, 2014-2021
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Include it to build kernel modules.
# REQ_VERSION_MAJOR and REQ_VERSION_PATCH must be defined beforehand.

$(if $(REQ_VERSION_MAJOR),,$(error You must define REQ_VERSION_MAJOR))
$(if $(REQ_VERSION_PATCH),,$(error You must define REQ_VERSION_PATCH))

ifeq ($(WITH_MODULES),no)
SKIP := 1
else
ifeq ($(LINUX_VERSION_MAJOR)$(LINUX_VERSION_PATCH),)
SKIP := 1
else
SKIP ?= $(shell \
	[ "$(LINUX_VERSION_MAJOR)" -gt "$(REQ_VERSION_MAJOR)" ] || \
	[ "$(LINUX_VERSION_MAJOR)" -eq "$(REQ_VERSION_MAJOR)" -a \
	  "$(LINUX_VERSION_PATCH)" -ge "$(REQ_VERSION_PATCH)" ]; echo $$?)
endif
endif

ifneq ($(SKIP),0)
MAKE_TARGETS := $(filter-out %.ko, $(MAKE_TARGETS))
endif

ifneq ($(filter install clean,$(MAKECMDGOALS)),)
MAKE_TARGETS := $(filter-out %.ko, $(MAKE_TARGETS))
MAKE_TARGETS += $(sort $(wildcard *.ko))
endif

CLEAN_TARGETS += .dep_modules *.mod built-in.a

CHECK_TARGETS := $(filter-out %.ko, $(CHECK_TARGETS))

MODULE_SOURCES := $(patsubst %.ko,%.c,$(filter %.ko, $(MAKE_TARGETS)))

# Ignoring the exit status of commands is done to be forward compatible with
# kernel internal API changes. The user-space test will return TCONF, if it
# doesn't find the module (i.e. it wasn't built either due to kernel-devel
# missing or module build failure).
%.ko: %.c .dep_modules ;

.dep_modules: $(MODULE_SOURCES)
	@echo "Building modules: $(MODULE_SOURCES)"
	-$(MAKE) -C $(LINUX_DIR) M=$(abs_srcdir)
	rm -rf *.mod.c *.o *.ko.unsigned modules.order .tmp* .*.ko .*.cmd Module.symvers
	@touch .dep_modules
