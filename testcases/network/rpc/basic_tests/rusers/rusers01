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

TCID="rusers01"
TST_TOTAL=5

TST_USE_LEGACY_API=1
. tst_net.sh

do_setup()
{
	tst_resm TINFO "Checking for rusersd on $(tst_ipaddr)"
	rpcinfo -u $(tst_ipaddr) rusersd > /dev/null 2>&1 || \
		tst_brkm TCONF "rusersd is inactive on $(tst_ipaddr)"
}

do_test()
{
	tst_resm TINFO "Test rusers with options set"

	EXPECT_RHOST_PASS rusers $(tst_ipaddr)

	local opts="-a -l"
	for opt in $opts; do
		EXPECT_RHOST_PASS rusers $opt $(tst_ipaddr)
	done

	tst_resm TINFO "Test rusers with bad options"
	EXPECT_RHOST_FAIL rusers bogushost
	EXPECT_RHOST_FAIL rusers -bogusflag $(tst_ipaddr)
}

do_setup
do_test

tst_exit
