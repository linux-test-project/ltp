#!/bin/sh
# Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it would be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
#
#  PURPOSE: Runs the fsx-linux tool with a 50000 iterations setting to
#	    attempt to uncover the "doread:read input/output" error
#	    received if the latest NFS patches for 2.4.17 from Trond
#	    are not applied. http://nfs.sf.net

TCID=nfsx
TST_TOTAL=1
TST_CLEANUP="nfs_cleanup"

. nfs_lib.sh
. test_net.sh

do_test()
{
	ITERATIONS=${ITERATIONS:=50000}
	tst_resm TINFO "starting fsx-linux -N $ITERATIONS..."
	fsx-linux -N $ITERATIONS testfile 2>&1 > fsx-out.log
	if [ "$?" -ne 0 ]; then
		tst_resm TFAIL "Errors have resulted from this test"
		cat fsx-out.log
	else
		tst_resm TPASS "fsx-linux test passed"
	fi
}

nfs_setup

do_test

tst_exit
