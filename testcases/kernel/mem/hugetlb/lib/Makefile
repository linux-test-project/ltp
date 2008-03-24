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

#########################################################################
# name of file	: Makefile						#
# description	: make(1) description file to build a library for the 	#
#		  common routines in the ipc(2) tests.			#
#########################################################################
SRCS=libipc.c
OBJS=$(SRCS:.c=.o)
MAINS=libipc.a
CFLAGS+=-I../../../../../include -g -Wall
CFLAGS+=-D_USC_LIB_

all: libipc.a

libipc.a: $(OBJS)
	$(AR) -rc $@ $(OBJS)

$(OBJS):
	$(CC) -c $(SRCS) $(CFLAGS)

install:

clean:
	/bin/rm -rf $(OBJS) libipc.a

