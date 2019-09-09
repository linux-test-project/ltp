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

TCID="ssh_stress03_rmt"
TST_TOTAL=1

. test.sh

# Check the arguments
if [ $# -ne 5 ]; then
	tst_brkm TBROK "Usage: $0 ipver rhost config port requests"
fi

ip_ver="$1"
server_ipaddr="$2"
ssh_config="$3"
rport="$4"
requests="$5"

ssh -$ip_ver -F $ssh_config $server_ipaddr \
	"true < /dev/null > /dev/null 2>&1" > /dev/null
[ $? -ne 0 ] && tst_brkm TBROK "Failed to connect '$server_ipaddr'"

lport=$(tst_get_unused_port ipv${ip_ver} stream)

# Set the ssh port-forwarding
case $ip_ver in
4)
	localhost="127.0.0.1"
	ssh -4 -f -N -L $lport:$server_ipaddr:$rport \
		root@$server_ipaddr -F $ssh_config
;;
6)
	localhost="::1"
	ssh -6 -f -N -L $lport:[$server_ipaddr]:$rport \
		root@$server_ipaddr -F $ssh_config
;;
esac

# Start the TCP traffic clients
netstress -r $requests -l -H $localhost -g $lport > /dev/null
ret=$?

# Stop the ssh port forwarding
all_conn=$(ps auxw | grep -Fv grep | \
	grep "ssh[[:blank:]].*${ssh_config}" | awk '{print $2}')
for ssh_pid in $all_conn ; do
	kill $ssh_pid
done

[ $ret -ne 0 ] && tst_brkm TBROK "TCP traffic client is dead"

# Check the connectivity again
ssh -$ip_ver -F $ssh_config $server_ipaddr \
	"true < /dev/null > /dev/null 2>&1" > /dev/null
[ $? -ne 0 ] && tst_brkm TBROK "Failed to connect '$server_ipaddr'"

tst_exit
