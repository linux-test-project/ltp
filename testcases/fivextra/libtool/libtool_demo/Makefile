###########################################################################
#  (C) Copyright IBM Corp. 2004                                           #
#                                                                         #
#  This program is free software;  you can redistribute it and/or modify  #
#  it under the terms of the GNU General Public License as published by   #
#  the Free Software Foundation; either version 2 of the License, or      #
#  (at your option) any later version.                                    #
#                                                                         #
#  This program is distributed in the hope that it will be useful,        #
#  but WITHOUT ANY WARRANTY;  without even the implied warranty of        #
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See              #
#  the GNU General Public License for more details.                       #
#                                                                         #
#  You should have received a copy of the GNU General Public License      #
#  along with this program;  if not, write to the Free Software           #
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA#
#                                                                         #
###########################################################################
# name of file	: Makefile						  #
# description	: make(1) description file for libtool tests		  #
###########################################################################

TARGETS= install-sh foo.h foo.c hello.c main.c

TGTDIR= libtool_demo

all:

install:
	mkdir -p ../../../bin/$(TGTDIR)
	for i in $(TARGETS) ; do cp -f $$i ../../../bin/$(TGTDIR)/$$i || break ; done ;

uninstall:
	rm -rf ../../../bin/$(TGTDIR)

clean:
