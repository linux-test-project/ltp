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
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

#########################################################################
# name of file	: Makefile						#
# description	: make(1) description for compiling the /ltp/testsuite	#
#		  tests. This Makefile would recursively call the 	#
#		  Makefiles inside each subdirectories, and, compiles	#
#		  and installs the tests.				#
#########################################################################
DEBUG_CFLAGS=-O
DEBUG_LDFLAGS=-s
CFLAGS+=-I../../include -g -Wall
LOADLIBES+=

SUBDIR = `ls */Makefile | sed "s/Makefile//g"` 

all:  
	@set -e; for i in lib $(SUBDIR); do \
		$(MAKE) -C $$i  all; \
	done

install:
	@set -e; for i in lib $(SUBDIR); do \
		$(MAKE) -C $$i  install; \
	done

clean:
	@set -e; for i in lib $(SUBDIR); do \
		$(MAKE) -C $$i  clean; \
	done

