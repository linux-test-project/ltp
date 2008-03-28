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
# name of file		 : Makefile		 		 		 		 		 		   #
# description		 : make file for the delete_module(2) testcases		 		   #
###########################################################################

EXTRA_CFLAGS+=		 -I../../../../include -Wall
MODCFLAGS+=		 /lib/modules/`uname -r`/build
LOADLIBES+=		 -L../../../../lib -lltp

obj-m := dummy_del_mod.o dummy_del_mod_dep.o
TARGETS=delete_module01 delete_module02 delete_module03
MODULES=dummy_del_mod.ko dummy_del_mod_dep.ko

all: $(TARGETS)
		 make -C $(MODCFLAGS) SUBDIRS=`pwd` modules
#		 chmod 755 $(OBJS)

delete_module01: delete_module01.c
		 $(CC) $(EXTRA_CFLAGS) delete_module01.c $(LOADLIBES) -o $@

delete_module02: delete_module02.c
		 $(CC) $(EXTRA_CFLAGS) delete_module02.c $(LOADLIBES) -o $@

delete_module03: delete_module03.c
		 $(CC) $(EXTRA_CFLAGS) delete_module03.c $(LOADLIBES) -o $@

install:
		 @set -e; for i in $(TARGETS); do ln -f $$i ../../../bin/$$i ; done
		 @set -e; for i in $(MODULES); do cp -f $$i /tmp/$$i ; done

clean:
		 rm -f $(TARGETS)
		 rm -rf .tmp_versions
		 rm -f /tmp/dummy*.ko
		 rm -f .dummy*
		 rm -f *.o
		 rm -f *.ko
		 rm -f *.mod.c
