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
# description	: make(1) description file for ipc(2) test directory.	#
#########################################################################
LTP_CFLAGS+=-I../../../../../include -g -Wall
LTP_LDFLAGS+=-L../../../../../lib -lltp
IPCTESTS=msgctl msgget msgsnd msgrcv semget semctl semop shmat shmdt shmctl shmget

all: $& $(IPCTESTS)
	for i in lib $(IPCTESTS); do \
		(cd $$i; $(MAKE) all) \
	done

clean:
	for i in lib $(IPCTESTS); do \
		(cd $$i; $(MAKE) clean) \
	done

install:
	for i in lib $(IPCTESTS); do \
		(cd $$i; $(MAKE) install) \
	done

strip_bin:
	for i in $(IPCTESTS); do \
		(cd $$i; $(MAKE) strip_bin) \
	done
