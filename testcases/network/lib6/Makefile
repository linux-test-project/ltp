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
# description	: make(1) description file for lib6 tests.	  	  #
###########################################################################
CFLAGS+=	-I../../../include -g
LOADLIBES+=	-L../../../lib runcc.a -lltp -lpthread

SRCS=$(wildcard *.c)
TARGETS= asapi_01 asapi_02 asapi_03 asapi_04 asapi_05 asapi_06 asapi_07 \
	getaddrinfo_01 in6_01 in6_02

all: runcc.a $(TARGETS)

install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../bin/$$i ; done

clean:
	rm -f $(TARGETS) *.a *.o

runcc.a: runcc.o
	ar cr runcc.a runcc.o
