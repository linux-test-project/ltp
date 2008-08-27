#
#  Copyright (c) SWSoft, 2007
#
#  This program is free software;  you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY;  without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#  the GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program;  if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

CFLAGS += -I../../../../include -Wall
LDLIBS += -L../../../../lib -lltp

SRCS    = $(wildcard *.c)
TARGETS = $(patsubst %.c,%,$(SRCS))
check_header = $(shell \
			if echo "\#include <$(1)>" | $(CC) -E - > /dev/null; \
			then echo -DHAS_SYS_INOTIFY ; \
			fi)

HAS_SYS_INOTIFY := $(call check_header,sys/inotify.h)
CFLAGS += $(HAS_SYS_INOTIFY)

all: $(TARGETS)

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../bin/$$i ; done

clean:
	rm -f $(TARGETS)
