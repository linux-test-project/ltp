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
# description	: make(1) description file for file command		  #
###########################################################################

# all executables in this directory are considered testcases
TARGETS=`find . -maxdepth 1 -perm +111 -type f`

all:	file-test-cprog file-tests.tar file-tests.tgz file-test.xxx.gz

file-tests.tar:
	tar cf file-tests.tar *

file-tests.tgz:
	tar cf file-tests.tgz *

file-test.xxx.gz:
	cp file-test-cprog.c file-test.xxx
	gzip file-test.xxx

install:
	for i in $(TARGETS) ; do ln -f $$i ../../bin/$$i || break ; done
	ln -f file-tests.tar ../../bin/
	ln -f file-tests.tgz ../../bin/
	ln -f file-test.xxx.gz ../../bin/

uninstall:
	for j in $(TARGETS) ; do rm -f ../../bin/$$j ; done
	rm ../../bin/file-tests.tar
	rm ../../bin/file-tests.tgz
	rm ../../bin/file-test.xxx.gz

clean:
	@rm -f *~ file-test-cprog file-tests.tar file-tests.tgz file-test.xxx.gz

