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

SRCS    = $(wildcard *.c)
TARGETS = $(patsubst %.c,%,$(SRCS))

check_header = $(shell \
	if echo "\#include <$(1)>" | $(CC) -E - > /dev/null 2>&1 ; \
	then echo yes ; \
	else echo no ; fi)

ifeq ($(call check_header,asm/ldt.h),yes)

NEEDSPECIAL:=$(shell echo "\#include <asm/ldt.h>" | $(CC) -E - 2>&1 | grep user_desc)
ifeq ($(strip $(NEEDSPECIAL)),)
CFLAGS+=-I../../../../include -g -Wall
else
CFLAGS+=-I../../../../include -g -Wall -DMODIFY_LDT_SPECIALCASE
endif

LDLIBS += -L../../../../lib -lltp

all: $(TARGETS)

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../bin/$$i ; done

else ### does not have asm/ldt.h ###

all install:

endif

clean:
	rm -f $(TARGETS)
