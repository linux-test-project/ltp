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
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#

DIR=lib
CFLAGS += -I../../../../include -Wall
CFLAGS += -D_USC_LIB_
SRC=libtestsuite.c
OBJS=$(SRC:.c=.o)
MAINS=../libtestsuite.a

all: ../libtestsuite.a

../libtestsuite.a: $(OBJS)
	$(AR) -rc $@ $(OBJS)

install:

clean:
	rm -f $(OBJS) ../libtestsuite.a
