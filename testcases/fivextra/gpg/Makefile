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
# History	29 Jun 2004 - rcp: removed CC=gcc to allow cross-biuilds.

###########################################################################
# name of file	: Makefile						  #
# description	: makefile fpr gpg.sh testcase				  #
###########################################################################

# all executables in this directory are considered testcases
TARGETS=`find . -maxdepth 1 -perm +111 -type f`

TESTDIR = checks
CFLAGS = -g -O2 -Wall
#LIBS = -lz -ldl -lnsl -lgdbm  -lnsl
LINK = $(CC) $(CFLAGS) -o $@
mk_tdata_SOURCES = mk-tdata.c
mk_tdata_OBJECTS = mk-tdata.o

mk-tdata: $(mk_tdata_OBJECTS)
	rm -f mk-tdata
	$(LINK) $(mk_tdata_OBJECTS) $(LIBS)

all: mk-tdata

install:
	for i in $(TARGETS) ; do cp -p $$i ../../bin/$$i || break ; done ;
	rm -rf ../../bin/$(TESTDIR) ;
	cp -a $(TESTDIR) ../../bin/$(TESTDIR) || break

uninstall:
	for i in $(TARGETS) ; do rm -f ../../bin/$$i ; done ;
	rm -rf ../../bin/$(TESTDIR)

clean:
	@rm -f mk-tdata $(mk_tdata_OBJECTS) *~ core

