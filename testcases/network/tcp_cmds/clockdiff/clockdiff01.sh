#!/bin/sh
# Copyright (c) 2016-2017 Oracle and/or its affiliates. All Rights Reserved.
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
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>

TCID=clockdiff01
TST_TOTAL=1
. test_net.sh

tst_require_root
tst_check_cmds cut clockdiff

tst_resm TINFO "check time delta for $(tst_ipaddr)"

output=$(clockdiff $(tst_ipaddr))

if [ $? -ne 0 ]; then
	tst_resm TFAIL "clockdiff failed to get delta"
else
	delta=$(echo "$output" | cut -d' ' -f2,3)
	if [ "$delta" = "0 0" ]; then
		tst_resm TPASS "delta is 0 as expected"
	else
		tst_resm TFAIL "not expected data received: '$output'"
	fi
fi

tst_exit
