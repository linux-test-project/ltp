#
#  (C) Copyright IBM Corp. 2003
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
# description	: make(1) description file for the lsof testcase	  #
###########################################################################

# all executables in this directory are considered testcases
TARGETS = `find . -maxdepth 1 -perm +111 -type f`
SUBDIRS = `find . -type d -maxdepth 1 ! -name "." ! -name CVS`

all:
	for i in $(SUBDIRS); do $(MAKE) -C $$i all || break ; done
	./patch_lsof.sh

install:	all
	for i in $(TARGETS) ; do ln -f $$i ../../bin/$$i || break ; done
	mkdir -p ../../bin/LT
	for d in $(SUBDIRS); do $(MAKE) -C $$d install || break ; done

uninstall:
	for j in $(TARGETS) ; do rm -f ../../bin/$$j ; done
	for d in $(SUBDIRS); do $(MAKE) -C $$d uninstall || break ; done
	rmdir ../../bin/LT

clean:
	@rm -f *~ testlist sedscript lsof.sh
	for i in $(SUBDIRS); do $(MAKE) -C $$i clean ; done

