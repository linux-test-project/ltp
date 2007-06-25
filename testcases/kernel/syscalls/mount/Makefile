#
#  Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
#  Copyright (C) International Business Machines  Corp., 2007
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
# HISTORY:
#  6/8/07 krisw@us.ibm.com  Copied change made in RHEL Makefile for seg fault.
#
#############################################################################

CFLAGS += -I../../../../include -Wall -D_GNU_SOURCE
LDLIBS += -L../../../../lib -lltp

SRCS    = $(wildcard *.c)
TARGETS = $(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

install: setuid_test.mode
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../bin/$$i ; done

clean:
	rm -f $(TARGETS)

setuid_test.mode:
	@/bin/sh ./setuid_test.mode.sh

.PHONY: setuid_test.mode
