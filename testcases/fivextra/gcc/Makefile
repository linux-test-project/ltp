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
# name of file  : Makefile                                                #
# description   : make(1) description file for gcc	                  #
###########################################################################

all:
	echo "Nothing to do compile,just make install to install the test cases"

install:
	ln -f gcc.sh  ../../bin/gcc.sh 
	chmod a+x ../../bin/gcc.sh
	(cd ../../bin ;rm -fr gcc-test; ln -sf  ../fivextra/gcc/gcc-test gcc-test)

uninstall:
	rm -f ../../bin/gcc.sh 
	rm -fr  ../../bin/gcc-test

clean:
	echo "Nothing to do clean"
