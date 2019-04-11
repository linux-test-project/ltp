# Copyright (c) 2017 Linux Test Project
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

top_srcdir		?= ../..

include $(top_srcdir)/include/mk/testcases.mk

CFLAGS			+= -D_GNU_SOURCE

stack_clash:	CFLAGS += -fno-optimize-sibling-calls

cve-2016-7042:	LDLIBS += $(KEYUTILS_LIBS)

cve-2016-7117:	CFLAGS += -pthread
cve-2016-7117:	LDLIBS += -lrt

cve-2014-0196:  CFLAGS += -pthread
cve-2014-0196:  LDLIBS += -lrt

ifneq ($(ANDROID),1)
cve-2014-0196:  LDLIBS += -lutil
endif

cve-2017-2671:	CFLAGS += -pthread
cve-2017-2671:	LDLIBS += -lrt

meltdown: CFLAGS += -I$(abs_srcdir)/../realtime/include

ifneq (,$(filter $(HOST_CPU),x86 x86_64))
meltdown: CFLAGS += -msse2
endif

cve-2017-17052:	CFLAGS += -pthread
cve-2017-17053:	CFLAGS += -pthread

cve-2015-3290:	CFLAGS += -pthread

include $(top_srcdir)/include/mk/generic_leaf_target.mk
