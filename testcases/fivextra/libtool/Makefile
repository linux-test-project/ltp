#
#  (C) Copyright IBM Corp. 2004
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
# description	: make(1) description file for libtool tests		  #
###########################################################################

TESTDIR=libtool_demo

TARGETS=`find . -maxdepth 1 -perm +111 -type f`

all:
	$(MAKE) -C $(TESTDIR)
	echo $(CC) > FIV_CC_NAME.txt

install:	all
	for i in $(TARGETS) FIV_CC_NAME.txt ; do cp -f $$i ../../bin || break ; done
	$(MAKE) -C $(TESTDIR) install

uninstall:
	for j in $(TARGETS) ; do rm -f ../../bin/$$j ; done
	$(MAKE) -C $(TESTDIR) uninstall

clean:
	@rm -f *~ FIV_CC_NAME.txt
	$(MAKE) -C $(TESTDIR) clean
