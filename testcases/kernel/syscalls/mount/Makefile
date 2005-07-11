#
# Copyright (c) Wipro Technologies Ltd, 2002.  All Rights Reserved.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of version 2 of the GNU General Public License as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it would be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write the Free Software Foundation, Inc., 59
# Temple Place - Suite 330, Boston MA 02111-1307, USA.
#
#
###########################################################################
# name of file	: Makefile						  #
# description	: make(1) description file for the mount(2) tests.	  #
###########################################################################
CFLAGS+=	-I../../../../include -D_GNU_SOURCE -Wall -g
CFLAGS+=	$(if $(UCLINUX),-DUCLINUX)
LDLIBS+=	-L../../../../lib -lltp
LDLIBS+=	$(if $(UCLINUX),-lc)

SRCS=$(wildcard mount*.c) setuid_test.c
TARGETS=$(patsubst %.c,%,$(SRCS))

all: $(TARGETS)

setuid_test.mode:
	@/bin/sh ./setuid_test.mode.sh

$(TARGETS): $(SRCS)
	$(CC) -o $@ $@.c $(CFLAGS) $(LDLIBS)

install: setuid_test.mode
	@set -e; for i in $(TARGETS) setuid_test; do ln -f $$i ../../../bin/$$i ; done

clean:
	rm -f $(TARGETS) setuid_test

.PHONY: setuid_test.mode
