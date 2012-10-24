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
# with this program; if not, write the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
###########################################################################
# name of file	: Makefile						  #
# description	: make file for the query_module(2) testcases		  #
###########################################################################

CFLAGS+=	-I../../../../include -Wall
MODCFLAGS+=	-I/lib/modules/$(shell uname -r)/build/include -Wall
LOADLIBES+=	-L../../../../lib -lltp

SRC1=dummy_query_mod.c dummy_query_mod_dep.c
OBJS=$(SRC1:.c=.o)
TARGETS=query_module01 query_module02 query_module03 $(OBJS)

all: $(TARGETS)
	chmod 755 $(OBJS)

$(OBJS): $(SRC1)
	$(CC) -DEXPORT_SYMTAB $(MODCFLAGS) -c $(SRC1) -Wall

query_module01: query_module01.c
	$(CC) $(CFLAGS) query_module01.c $(LOADLIBES) -o $@

query_module02: query_module02.c
	$(CC) $(CFLAGS) query_module02.c $(LOADLIBES) -o $@

query_module03: query_module03.c
	$(CC) $(CFLAGS) query_module03.c $(LOADLIBES) -o $@

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../../bin/$$i ; done

clean:
	rm -f $(TARGETS)

