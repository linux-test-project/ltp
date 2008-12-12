################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2007                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################

CFLAGS += -Wall
CPPFLAGS += -I../../../../include -I../libclone
LDLIBS += -L../../../../lib -L../libclone ../libclone/libclone.a -lltp -lrt

SRCS    := $(wildcard *.c)
TARGETS := $(SRCS:%.c=%)

HAS_UNSHARE ?= $(shell ../check_for_unshare && echo y)
ifneq ($(HAS_UNSHARE),y)
TARGETS :=
endif

all: $(TARGETS)

clean:
	rm -f $(TARGETS) *.o

install:
ifeq ($(HAS_UNSHARE),y)
	@set -e; for i in $(TARGETS) runpidnstest.sh check_pidns_enabled; do ln -f $$i ../../../bin/$$i ; chmod +x runpidnstest.sh ; done
endif
