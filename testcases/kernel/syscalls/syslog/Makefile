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

CFLAGS += -I../../../../include -Wall
LDLIBS += -L../../../../lib -lltp

SRCS    = $(wildcard *.c)
TARGETS = $(patsubst %.c,%,$(SRCS))
SCRIPTS = syslog01 syslog02 syslog03 syslog04 syslog05 syslog06 syslog07 syslog08 syslog09 syslog10

all: $(TARGETS)

install:
	@set -e; for i in $(TARGETS) $(SCRIPTS); do ln -f $$i ../../../bin/$$i ; done

clean:
	rm -f $(TARGETS)
