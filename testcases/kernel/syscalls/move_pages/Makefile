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

CFLAGS += -I../../../../include -Wall
LDLIBS += -L../../../../lib -lltp -lnuma -lrt

ifeq ($(HAS_NUMA),yes)
SRCS    = $(wildcard *.c)
TARGETS = $(patsubst %.c, %, $(wildcard *[0-9].c))
endif

all: $(TARGETS)

move_pages_support.o: move_pages_support.h move_pages_support.c

$(TARGETS): move_pages_support.o

install: move_page03.mode move_page11.mode
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../bin/$$i ; done
	chmod 755 move_pages.sh; cp move_pages.sh ../../../bin/.

clean:
	rm -f $(TARGETS)

move_page03.mode:
	@/bin/sh ./move_pages03.mode.sh

move_page11.mode:
	@/bin/sh ./move_pages11.mode.sh

.PHONY: move_pages03.mode