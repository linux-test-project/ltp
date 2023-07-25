#
#    Environment post-setup Makefile.
#
#    Copyright (c) Linux Test Project, 2009-2020
#    Copyright (c) Cisco Systems Inc., 2009
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
# Ngie Cooper, July 2009
#

ENV_PRE_LOADED			?= $(error You must load env_pre.mk before including this file)

include $(top_srcdir)/include/mk/functions.mk

ifndef ENV_POST_LOADED
ENV_POST_LOADED = 1

# Default source search path. Modify as necessary, but I would call that
# poor software design if you need more than one search directory, and
# would suggest creating a general purpose static library to that end.
vpath %.c $(abs_srcdir)
vpath %.S $(abs_srcdir)

# For config.h, et all.
CPPFLAGS			+= -I$(top_srcdir)/include -I$(top_builddir)/include -I$(top_srcdir)/include/old/

LDFLAGS				+= -L$(top_builddir)/lib

ifeq ($(ANDROID),1)
LDFLAGS				+= -L$(top_builddir)/lib/android_libpthread
LDFLAGS				+= -L$(top_builddir)/lib/android_librt
endif

MAKE_TARGETS			?= $(notdir $(patsubst %.c,%,$(sort $(wildcard $(abs_srcdir)/*.c))))
MAKE_TARGETS			:= $(filter-out $(FILTER_OUT_MAKE_TARGETS),$(MAKE_TARGETS))

# with only *.dwo, .[0-9]+.dwo can not be cleaned
CLEAN_TARGETS			+= $(MAKE_TARGETS) $(HOST_MAKE_TARGETS) *.o *.pyc .cache.mk *.dwo .*.dwo

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

$(abspath $(addprefix $(DESTDIR)/$(INSTALL_DIR)/,$(sort $(dir $(INSTALL_TARGETS) $(MAKE_TARGETS))))):
	mkdir -p "$@"
$(foreach install_target,$(INSTALL_TARGETS),$(eval $(call generate_install_rule,$(install_target),$(abs_srcdir),$(INSTALL_DIR))))
$(foreach make_target,$(MAKE_TARGETS),$(eval $(call generate_install_rule,$(make_target),$(abs_builddir),$(INSTALL_DIR))))

else  # else ! $(filter-out install,$(MAKECMDGOALS)),$(MAKECMDGOALS)
$(error You must define $$(prefix) before executing install)
endif # END $(filter-out install,$(MAKECMDGOALS)),$(MAKECMDGOALS)
endif

CHECK_TARGETS			?= $(addprefix check-,$(notdir $(patsubst %.c,%,$(sort $(wildcard $(abs_srcdir)/*.c)))))
CHECK_TARGETS			:= $(filter-out $(addprefix check-, $(FILTER_OUT_MAKE_TARGETS)), $(CHECK_TARGETS))
CHECK_HEADER_TARGETS		?= $(addprefix check-,$(notdir $(sort $(wildcard $(abs_srcdir)/*.h))))
CHECK				?= $(abs_top_srcdir)/tools/sparse/sparse-ltp
CHECK_NOFLAGS			?= $(abs_top_srcdir)/scripts/checkpatch.pl -f --no-tree --terse --no-summary --ignore CONST_STRUCT,VOLATILE,SPLIT_STRING
SHELL_CHECK			?= $(abs_top_srcdir)/scripts/checkbashisms.pl --force --extra
SHELL_CHECK_TARGETS		?= $(addprefix check-,$(notdir $(sort $(wildcard $(abs_srcdir)/*.sh))))

ifeq ($(CHECK),$(abs_top_srcdir)/tools/sparse/sparse-ltp)
CHECK_DEPS			+= $(CHECK)
endif

include $(top_srcdir)/include/mk/rules.mk

endif
