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
CFLAGS+=	-Wall -I../../../../include -D _GNU_SOURCE
LDFLAGS+=	-L../../../../lib -lltp

SRCS=$(wildcard *.c)
TARGETS=$(patsubst %.c,%,$(SRCS))
TEST=sigpending01

all: $(TARGETS)

$(TARGETS): $(SRCS)
	$(CC) -o $@ $@.c $(CFLAGS) $(LDFLAGS)
	@chmod 755 $(TEST)

install: all
	@set -e; for i in $(TARGETS) $(TEST); do ln -f $$i ../../../bin/$$i ; done

clean:
	rm -f $(TARGETS)


