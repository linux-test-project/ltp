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
# description	: makefile for e2fsprogs testcase			  #
###########################################################################

# all executables in this directory are considered testcases
TARGETS=`find . -maxdepth 1 -perm +111 -type f`
DIRS=d_loaddump \
	f_bad_local_jnl \
	f_baddir \
	f_baddotdir \
	f_badprimary \
	f_bitmaps \
	f_dirlink \
	f_end-bitmap \
	f_ext_journal \
	f_holedir \
	f_journal \
	f_okgroup \
	f_orphan \
	f_preen \
	f_reconnect \
	f_swapfs \
	f_zero_group \
	f_zero_super

FILES=run_e2fsck test_config
SETUP=defaults

all:

install:
	@for i in $(TARGETS) $(FILES) ; do cp -p $$i ../../bin/$$i || break ; done ;
	@for j in $(DIRS) $(SETUP) ; do cp -a $$j ../../bin/$$j || break ; done

uninstall:
	@for i in $(TARGETS) $(FILES) ; do rm -f ../../bin/$$i ; done ;
	@for j in $(DIRS) $(SETUP) ; do rm -rf ../../bin/$$j || break ; done

clean:
	@rm -f *~ *.log *.ok *.new

