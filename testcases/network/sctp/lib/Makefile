#
# (C) Copyright IBM Corp. 2003
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
# description	: make(1) description file to build SCTP library        #
#		                                                        # 
#########################################################################
INCLUDES+=-I../include

CFLAGS += -g $(INCLUDES)
SRCS=addrs.c bindx.c opt_info.c peeloff.c recvmsg.c sendmsg.c connectx.c
OBJS=$(SRCS:.c=.o)
MAINS=libsctp.a

all: libsctp.a

libsctp.a: $(OBJS)
	$(AR) -rc $@ $(OBJS)

$(OBJS): $(SRCS) ../include/netinet/sctp.h 
	$(CC) -c $(SRCS) $(CFLAGS)

install:

clean:
	/bin/rm -rf $(OBJS) libsctp.a
