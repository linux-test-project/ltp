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

SUBDIRS := libclone utsname sysvipc pidns netns

all: check_for_unshare
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i $@; done

check_for_unshare: check_for_unshare.c
	$(CC) $(CFLAGS) -rdynamic -o $@ $< ../../../lib/tst_kvercmp.c -I../../../include -ldl

install: check_for_unshare
	@set -e; ln -f check_for_unshare ../../bin/check_for_unshare; \
	ln -f container_test.sh ../../bin/container_test.sh; \
	for i in $(SUBDIRS); do $(MAKE) -C $$i $@ ; done; \
	chmod ugo+x container_test.sh

clean:
	@set -e; for i in $(SUBDIRS); do $(MAKE) -C $$i $@ ; done
	rm -f check_for_unshare
