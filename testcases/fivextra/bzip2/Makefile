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
# description	: make(1) description file for the bzip2/bunzip2 test.    #
###########################################################################

# all executables in this directory are considered testcases
TARGETS=`find . -maxdepth 1 -perm +111 -type f`

all:

install:
	for i in $(TARGETS) ; do ln -f $$i ../../bin/$$i || break ; done
	ln -f sample1.ref ../../bin/sample1.ref || break
	ln -f sample2.ref ../../bin/sample2.ref || break
	ln -f sample3.ref ../../bin/sample3.ref || break
	ln -f sample1.bz2 ../../bin/sample1.bz2 || break
	ln -f sample2.bz2 ../../bin/sample2.bz2 || break
	ln -f sample3.bz2 ../../bin/sample3.bz2 || break

uninstall:
	for j in $(TARGETS) ; do rm -f ../../bin/$$j ; done
	rm -f ../../bin/sample1.ref
	rm -f ../../bin/sample2.ref
	rm -f ../../bin/sample3.ref
	rm -f ../../bin/sample1.bz2
	rm -f ../../bin/sample2.bz2
	rm -f ../../bin/sample3.bz2

clean:
	@rm -f *~

