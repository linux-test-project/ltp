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
# description	: make(1) description file for the ipc(2) shmget tests.	  #
###########################################################################
MAINS=shmget01 shmget02 shmget03 shmget04 shmget05
SRCS=$(MAINS:=.c)
OBJS=$(MAINS:=.o)
IPC_LDFLAGS=-L.. -lipc
LTP_CFLAGS=-I../../../../../include -g

all: $& $(MAINS)

$(MAINS): $(SRCS)
	$(CC) -o $@ $@.c $(LTP_CFLAGS) $(LTP_LDFLAGS) $(IPC_LDFLAGS)

clean:
	/bin/rm -rf $(OBJS) $(MAINS)

install: all
	@for i in $(MAINS); do ln -f $$i ../../../../bin/$$i ; done

strip_bin:
	strip $(MAINS) > /dev/null 2>&1
