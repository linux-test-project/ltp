################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2008                 ##
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
## Author:      Veerendra <veeren@linux.vnet.ibm.com>                         ##
################################################################################

CFLAGS += -Wall
CPPFLAGS += -I../../../../include -I../libclone
LDLIBS += -L../../../../lib -L../libclone ../libclone/libnetns.a -lltp
SCRIPTS = initialize.sh parentns.sh childns.sh                  \
parent.sh child.sh                                              \
parent_view.sh parent_share.sh  child_propagate.sh              \
parent_1.sh parent_2.sh child_1.sh child_2.sh                   \
delchild.sh rename_net.sh                                       \
paripv6.sh childipv6.sh                                         \
par_ftp.sh ch_ftp.sh container_ftp.pl                           \
runnetnstest.sh nw_under_ns.sh

SRCS    = $(wildcard *.c)
TARGETS = $(SRCS:%.c=%)

HAS_UNSHARE ?= $(shell ../check_for_unshare && echo y)
ifneq ($(HAS_UNSHARE),y)
TARGETS :=
endif

all: $(TARGETS)
	@chmod +x $(SCRIPTS)

install:
ifeq ($(HAS_UNSHARE),y)
	for i in $(TARGETS) $(SCRIPTS) runnetnstest.sh check_netns_enabled; do ln -f $$i ../../../bin/$$i ; chmod +x runnetnstest.sh ; done
endif

clean:
	rm -f $(TARGETS) *.o
