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

TCID="ssh_stress02_rmt"
TST_TOTAL=1

. test.sh

# Check the arguments
if [ $# -ne 5 ]; then
	tst_brkm TBROK "Usage: $0 ipver rhost config connections duration"
fi

ip_ver="$1"
server_ipaddr="$2"
ssh_config="$3"
connections="$4"
duration="$5"

ssh -$ip_ver -F $ssh_config $server_ipaddr \
	"true < /dev/null > /dev/null 2>&1" > /dev/null
[ $? -ne 0 ] && tst_brkm TBROK "Failed to connect '$server_ipaddr'"

start_epoc=$(date +%s)
while true ; do
	# Exit when the specified seconds have passed.
	current_epoc=$(date +%s)
	elapse_epoc=$(($current_epoc - $start_epoc))

	[ $elapse_epoc -ge $duration ] && break

	# Do not make ssh connection over the specified quantity
	ssh_num=$(jobs | wc -l)
	if [ $ssh_num -ge $connections ]; then
		sleep 1
		continue;
	fi

	# specified wait time and login time
	wait_sec=$(($(od -A n -d -N 1 /dev/random) * 3 / 255))
	login_sec=$(($(od -A n -d -N 1 /dev/random) * 10 / 255))

	# Login to the server
	(sleep $wait_sec ; ssh -$ip_ver -F $ssh_config -l root $server_ipaddr \
		"sleep $login_sec < /dev/null > /dev/null 2>&1") > \
		/dev/null 2>&1 &
done

# wait for the finish of all process
wait

# Check the connectivity again
ssh -$ip_ver -F $ssh_config $server_ipaddr \
	"true < /dev/null > /dev/null 2>&1" > /dev/null
[ $? -ne 0 ] && tst_brkm TBROK "Failed to connect '$server_ipaddr'"

tst_exit
