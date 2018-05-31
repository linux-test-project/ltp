#!/bin/sh

# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2005
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
# along with this program; if not, write the Free Software Foundation,
# Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# Author: Mitsuru Chinen <mitch@jp.ibm.com>
#

TCID="ssh_stress01_rmt"
TST_TOTAL=1

. test.sh

if [ $# -ne 4 ]; then
	tst_brkm TBROK "Usage: $0 ipver rhost config connections"
fi

ip_ver="$1"
server_ipaddr="$2"
ssh_config="$3"
connections="$4"

ssh -$ip_ver -F $ssh_config $server_ipaddr \
	"true < /dev/null > /dev/null 2>&1" > /dev/null

[ $? -ne 0 ] && tst_brkm TBROK "Can't connect to '$server_ipaddr'"

# Make ssh connections
num=0
while [ $num -lt $connections ]; do
	ssh -$ip_ver -f -N -F $ssh_config $server_ipaddr
	if [ $? -ne 0 ]; then
		tst_resm TINFO "'$num' seems the max num of ssh conn"
		break
	fi
	num=$(($num + 1))
done

# Disconnect all ssh connection
all_conn=$(ps auxw | grep -Fv grep | \
	grep "ssh[[:blank:]].*${ssh_config}" | awk '{print $2}')
kill $all_conn

# Check the connectivity again
ssh -$ip_ver -F $ssh_config $server_ipaddr \
	"true < /dev/null > /dev/null 2>&1" > /dev/null
if [ $? -ne 0 ]; then
	tst_brkm TBROK "Failed to connect $server_ipaddr"
fi

tst_exit
