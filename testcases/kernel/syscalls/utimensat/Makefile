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

all:
	@set -e; $(MAKE) check_for_utimensat_support; \
	if './check_for_utimensat_support' > /dev/null 2>&1; then \
		$(MAKE) utimensat01; \
	else echo "System does not support utimensat syscall support"; true; fi

check_for_utimensat_support: check_for_utimensat_support.c
	$(CC) $(CFLAGS) -o $@ $< ../../../../lib/tst_kvercmp.c -I../../../../include

install:
	@set -e; \
	chmod 755 utimensat_tests.sh; \
	ln -f utimensat_tests.sh ../../../bin/utimensat_tests.sh; \
	if './check_for_utimensat_support' > /dev/null 2>&1; then \
		for i in $(TARGETS); do ln -f $$i ../../../bin/$$i ; done \
	else echo "System does not support utimensat syscall support"; true; fi

clean:
	rm -f $(TARGETS)
	rm -rf check_for_utimensat_support
