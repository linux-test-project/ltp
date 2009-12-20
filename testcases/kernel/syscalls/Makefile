#
#    testcases/kernel/syscalls Makefile.
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

top_srcdir		?= ../../..

include	$(top_srcdir)/include/mk/env_pre.mk

# libevent requires some massaging to get this to work.
FILTER_OUT_DIRS		:= libevent
# pcllib is busted as all heck because of libtool
FILTER_OUT_DIRS		+= pcllib

ifeq ($(wildcard $(abs_srcdir)/libevent/test/Makefile),)
$(warning libevent/test/Makefile does not exist; skipping to avoid compile errors)
$(warning Please run "make autotools" and "configure" at the top-level LTP)
$(warning directory if you want to run this test)
else

LIBEVENT		:= libevent/lib/libevent.a

CLEAN_DEPS		:= libevent-clean

CLEAN_TARGETS		:= $(LIBEVENT)

INSTALL_DEPS		:= libevent-install

INSTALL_TARGETS		:= libevent/run_libevent.sh libevent/test/test-libevent.sh

LIBEVENT_APPS		:= regress test-eof test-init test-time test-weof

TEST_APPS		:= $(addprefix libevent/test/,$(LIBEVENT_APPS))

MAKE_DEPS		:= libevent-all

MAKE_TARGETS		+= $(TEST_APPS)

libevent libevent/test: %:
	mkdir -p "$@"

$(TEST_APPS): CPPFLAGS += -I$(abs_srcdir)/libevent
$(TEST_APPS): LDFLAGS += -L$(abs_builddir)/libevent
$(TEST_APPS): LDLIBS += -levent

$(LIBEVENT): $(abs_srcdir)/libevent/Makefile libevent
	$(MAKE) -C libevent -f "$(firstword $^)" libevent.a

libevent-all: $(abs_srcdir)/libevent/test/Makefile | $(LIBEVENT) libevent/test
	$(MAKE) -C libevent/test -f "$(firstword $^)" $(LIBEVENT_APPS)

libevent-clean libevent-install: libevent-%: $(abs_srcdir)/libevent/test/Makefile | libevent/test
	$(MAKE) -C libevent/test -f "$(firstword $^)" $*

endif

ifeq ($(UCLINUX),1)
FILTER_OUT_DIRS	+= capget capset chmod chown clone fork getcontext llseek \
		   nftw profil remap_file_pages
endif

include $(top_srcdir)/include/mk/generic_trunk_target.mk
