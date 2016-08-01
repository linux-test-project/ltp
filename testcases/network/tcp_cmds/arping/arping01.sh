#!/bin/sh
# Copyright (c) 2016 Oracle and/or its affiliates. All Rights Reserved.
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

TCID=arping01
TST_TOTAL=1
. test_net.sh

tst_require_root
tst_check_cmds arping

timeout="10"
ip_addr=$(tst_ipaddr rhost)
dev=$(tst_iface)

tst_resm TINFO "arping host '$RHOST' with ip '$ip_addr' dev '$dev'"
tst_resm TINFO "with timeout '$timeout' seconds"

arping -w $timeout "$ip_addr" -I $dev -fq

if [ $? -ne 0 ]; then
	tst_resm TFAIL "arping to '$RHOST' failed"
else
	tst_resm TPASS "done"
fi

tst_exit
