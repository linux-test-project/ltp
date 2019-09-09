#!/bin/sh
# Copyright (c) 2017 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2000
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

TCID="rup01"
TST_TOTAL=7

TST_USE_LEGACY_API=1
. tst_net.sh

do_setup()
{
	tst_resm TINFO "Checking for rstatd on $(tst_ipaddr)"
	rpcinfo -u $(tst_ipaddr) rstatd 3 > /dev/null 2>&1 || \
		tst_brkm TCONF "rstatd is inactive on $(tst_ipaddr)"
}

do_test()
{
	tst_resm TINFO "Test rup with options set"

	EXPECT_RHOST_PASS rup $(tst_ipaddr)

	local opts="-d -h -l -t"
	for opt in $opts; do
		EXPECT_RHOST_PASS rup $opt $(tst_ipaddr)
	done

	tst_resm TINFO "Test rup with bad options"
	EXPECT_RHOST_FAIL rup bogushost
	EXPECT_RHOST_FAIL rup -bogusflag $(tst_ipaddr)
}

do_setup
do_test

tst_exit
