#!/bin/sh
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
################################################################################

export LTPROOT=${LTPROOT:=$(readlink -f "$(dirname "$0")")}
export TCID=libevent01
export TST_TOTAL=1
export TST_COUNT=0

. cmdlib.sh

tst_setup
if ! is_root ; then
	tst_resm TCONF "You need to be root to run these tests"
	TST_EXIT=0
else
	"$LTPROOT/testcases/bin/libevent/test/test-libevent.sh"
	TST_EXIT=$?
fi
tst_cleanup
