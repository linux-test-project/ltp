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
###########################################################################
# name of file	: Makefile						  #
# description	: make file for the delete_module(2) testcases		  #
###########################################################################

CFLAGS+=	-I../../../../include -Wall
LOADLIBES+=	-L../../../../lib -lltp

SRC1=dummy_del_mod.c dummy_del_mod_dep.c
OBJS=$(SRC1:.c=.o)
TARGETS=delete_module01 delete_module02 delete_module03 $(OBJS)

all: $(TARGETS)

$(OBJS): $(SRC1)
	$(CC) -c $(SRC1) -Wall 
#	$(CC) -c $(SRC1) -Wall -DMODULE -D__KERNEL__
# Removed defintions, b/c they are already set in source files.
# -11/22/02 Robbie Williamson <robbiew@us.ibm.com>     

delete_module01: delete_module01.c
	$(CC) $(CFLAGS) delete_module01.c $(LOADLIBES) -o $@

delete_module02: delete_module02.c
	$(CC) $(CFLAGS) delete_module02.c $(LOADLIBES) -o $@

delete_module03: delete_module03.c
	$(CC) $(CFLAGS) delete_module03.c $(LOADLIBES) -o $@

install:
	@for i in $(TARGETS); do ln -f $$i ../../../bin/$$i ; done

clean:
	rm -f $(TARGETS)

