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
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

###########################################################################
# name of file	: Makefile						  #
###########################################################################
CFLAGS+=	-I../../../../include -D_GNU_SOURCE -Wall
CFLAGS+=	$(if $(UCLINUX),-DUCLINUX)
LDLIBS+=		 -L../../../../lib -lltp
LDLIBS+=		$(if $(UCLINUX),-lc)

SRCS=$(wildcard fchmod*.c) change_owner.c
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

change_owner.mode: 
	@/bin/sh ./change_owner.mode.sh

$(TARGETS): $(SRCS)
		 $(CC) -o $@ $@.c $(CFLAGS) $(LDLIBS)


install: change_owner.mode
	@set -e; for i in $(TARGETS) change_owner; do ln -f $$i ../../../bin/$$i ; done

clean:
	rm -f $(TARGETS) change_owner

.PHONY: change_owner.mode
