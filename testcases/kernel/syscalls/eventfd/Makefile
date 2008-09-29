#
#  Copyright (c) International Business Machines  Corp., 2001
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

include ../utils/cond.mk


CFLAGS += -I../../../../include -Wall
LDLIBS += -L../../../../lib -lltp

check_aio_eventfd = $(shell \
	if echo -e \
	"\#include <libaio.h>\n\
	int main() \
	{ \
	io_set_eventfd(NULL, 0); \
	return 0; \
	}" | $(CC) -xc -laio - > /dev/null 2>&1 ; \
	then echo yes ; \
	else echo no ; fi)

HAS_LIBAIO = $(call check_header,libaio.h)
HAS_LIBAIO_EVENTFD = $(call check_aio_eventfd)

ifeq ($(HAS_LIBAIO_EVENTFD),yes)
CFLAGS += -DHAS_AIO_EVENTFD
LDLIBS += -laio
endif

SRCS    = $(wildcard *.c)
TARGETS = $(patsubst %.c, %, $(wildcard *.c))

all: $(TARGETS)
ifeq ($(HAS_LIBAIO),no)
	@echo "Note: Libaio is required for eventfd overflow testing.";
else
ifeq ($(HAS_LIBAIO_EVENTFD),no)
	@echo "Note: Eventfd support is required in libaio for overflow testing.";
endif
endif

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../bin/$$i ; done

clean:
	rm -f $(TARGETS)

