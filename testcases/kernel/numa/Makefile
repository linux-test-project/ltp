#
#  Copyright (c) International Business Machines  Corp., 2007
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

CFLAGS += -I../../../include -Wall -DNUMA_VERSION1_COMPATIBILITY

LDLIBS += -L../../../lib -lltp 

ifeq ($(HAS_NUMA),yes)    #its a numa machine
LDLIBS += -L../../../lib  -lnuma
SRCS    = $(wildcard *.c)
TARGETS = $(patsubst %.c,%,$(SRCS))
else                                    #its not a numa machine
SRCS    = support_numa.c
TARGETS = $(patsubst %.c,%,$(SRCS))
endif

all: $(TARGETS) 


install:
	@set -e; for i in $(TARGETS); do ln -f $$i ../../bin/$$i ; done
	chmod 755 numa01.sh; cp numa01.sh ../../bin/.

clean:
	rm -f $(TARGETS)
