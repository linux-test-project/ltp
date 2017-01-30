#
#    Environment post-setup Makefile.
#
#    Copyright (C) 2009, Cisco Systems Inc.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Garrett Cooper, July 2009
#

ENV_PRE_LOADED			?= $(error You must load env_pre.mk before including this file)

include $(top_srcdir)/include/mk/functions.mk

ifndef ENV_POST_LOADED
ENV_POST_LOADED = 1

# Default source search path. Modify as necessary, but I would call that
# poor software design if you need more than one search directory, and
# would suggest creating a general purpose static library to that end.
vpath %.c $(abs_srcdir)

# For config.h, et all.
CPPFLAGS			+= -I$(top_srcdir)/include -I$(top_builddir)/include -I$(top_srcdir)/include/old/

LDFLAGS				+= -L$(top_builddir)/lib

ifeq ($(UCLINUX),1)
CPPFLAGS			+= -D__UCLIBC__ -DUCLINUX
endif

ifeq ($(ANDROID),1)
# There are many undeclared functions, it's best not to accidentally overlook
# them.
CFLAGS				+= -Werror-implicit-function-declaration

LDFLAGS				+= -L$(top_builddir)/lib/android_libpthread
LDFLAGS				+= -L$(top_builddir)/lib/android_librt
endif

MAKE_TARGETS			?= $(notdir $(patsubst %.c,%,$(wildcard $(abs_srcdir)/*.c)))

MAKE_TARGETS			:= $(filter-out $(FILTER_OUT_MAKE_TARGETS),$(MAKE_TARGETS))

CLEAN_TARGETS			+= $(MAKE_TARGETS) *.o *.pyc

# Majority of the files end up in testcases/bin...
INSTALL_DIR			?= testcases/bin

ifneq ($(filter-out install,$(MAKECMDGOALS)),$(MAKECMDGOALS))

ifeq ($(strip $(INSTALL_DIR)),)
INSTALL_DIR			:= $(error You must define INSTALL_DIR before including this file)
endif

ifneq ($(strip $(prefix)),)
# Value specified by INSTALL_DIR isn't an absolute path, so let's tack on $(prefix).
ifneq ($(patsubst /%,,$(INSTALL_DIR)),)
INSTALL_DIR			:= $(prefix)/$(INSTALL_DIR)
endif

# Glob any possible expressions, but make sure to zap the $(abs_srcdir)
# reference at the start of the filename instead of using $(notdir), so that
# way we don't accidentally nuke the relative path from $(abs_srcdir) that
# may have been set in the Makefile.
INSTALL_TARGETS			:= $(wildcard $(addprefix $(abs_srcdir)/,$(INSTALL_TARGETS)))
INSTALL_TARGETS			:= $(patsubst $(abs_srcdir)/%,%,$(INSTALL_TARGETS))

# The large majority of the files that we install are going to be apps and
# scripts, so let's chmod them like that.
INSTALL_MODE			?= 00775

ifdef MAKE_3_80_COMPAT

INSTALL_PATH			:= $(call MAKE_3_80_abspath,$(DESTDIR)/$(INSTALL_DIR))

INSTALL_TARGETS_ABS		:= $(call MAKE_3_80_abspath,$(addprefix $(INSTALL_PATH)/,$(INSTALL_TARGETS)))
MAKE_TARGETS_ABS		:= $(call MAKE_3_80_abspath,$(addprefix $(INSTALL_PATH)/,$(MAKE_TARGETS)))

INSTALL_FILES			:= $(INSTALL_TARGETS_ABS) $(MAKE_TARGETS_ABS)

$(INSTALL_TARGETS_ABS):
	test -d "$(@D)" || mkdir -p "$(@D)"
	install -m $(INSTALL_MODE) "$(abs_srcdir)/$(subst $(INSTALL_PATH)/,,$@)" "$@"

$(MAKE_TARGETS_ABS):
	test -d "$(@D)" || mkdir -p "$(@D)"
	install -m $(INSTALL_MODE) "$(abs_builddir)/$(subst $(INSTALL_PATH)/,,$@)" "$@"
else
$(abspath $(addprefix $(DESTDIR)/$(INSTALL_DIR)/,$(sort $(dir $(INSTALL_TARGETS) $(MAKE_TARGETS))))):
	mkdir -p "$@"
$(foreach install_target,$(INSTALL_TARGETS),$(eval $(call generate_install_rule,$(install_target),$(abs_srcdir),$(INSTALL_DIR))))
$(foreach make_target,$(MAKE_TARGETS),$(eval $(call generate_install_rule,$(make_target),$(abs_builddir),$(INSTALL_DIR))))
endif

else  # else ! $(filter-out install,$(MAKECMDGOALS)),$(MAKECMDGOALS)
$(error You must define $$(prefix) before executing install)
endif # END $(filter-out install,$(MAKECMDGOALS)),$(MAKECMDGOALS)
endif

endif
