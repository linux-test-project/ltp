#!/bin/sh
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
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
# along with this program; if not, write the Free Software Foundation,
# Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# TEST DESCRIPTION:
#     To verify that IP Multicast can be used to send UDP datagrams
#     between two or more nodes on the same subnetwork using
#     a specific IP Multicast group and a specific port address.
#
# Authors:
#     Robbie Williamson (robbiew@us.ibm.com)
#     Michael Reed mreedltp@vnet.ibm.com

TTL=10
OUTFILE=mc_commo_out

TCID=mc_commo
TST_TOTAL=2
TST_CLEANUP=do_cleanup
TST_USE_LEGACY_API=1

do_setup()
{
	tst_require_cmds netstat pgrep

	OCTET=$(ps -ewf | grep [m]c_commo | wc -l | awk '{print $1}')
	GROUP_ADDR=224.0.0.$OCTET
	PORT=$(tst_get_unused_port ipv4 dgram)
	tst_tmpdir
}

do_test()
{
	# Run setsockopt/getsockopt test
	# Start up the recv on local host
	tst_resm TINFO "Start mc_recv $GROUP_ADDR $(tst_ipaddr) $PORT"
	mc_recv $GROUP_ADDR $(tst_ipaddr) $PORT >> $OUTFILE &
	SERVER_PID=$!
	sleep 5
	pgrep -x mc_recv > /dev/null || \
		tst_brkm TFAIL "Could NOT start mc_recv on $(tst_ipaddr)"

	grep -q "cannot join group" $OUTFILE && \
		tst_brkm TFAIL "Membership NOT set"

	netstat -ng | grep -q $GROUP_ADDR || \
		tst_brkm TFAIL "membership not set for $GROUP_ADDR"

	tst_resm TINFO "Start on $RHOST mc_send $GROUP_ADDR" \
	               "$(tst_ipaddr rhost) $PORT $TTL"

	tst_rhost_run -b -c "mc_send $GROUP_ADDR $(tst_ipaddr rhost) $PORT $TTL"
	sleep 10
	tst_rhost_run -s -c "pgrep -x mc_send > /dev/null"

	tst_resm TINFO "Waiting for 100 sec.! Do not interrupt."
	sleep 100  #wait until datagrams are received in $STATUS

	#test if datagrams has been sent
	grep -q "$(tst_ipaddr rhost) [0-9] [0-9]" $OUTFILE || \
		tst_brkm TFAIL "NO Datagrams received from $RHOST"

	tst_resm TPASS "Test Successful"
}

do_cleanup()
{
	ps -p $SERVER_PID > /dev/null 2>&1
	if [ $? -eq 0 ]; then
		kill -9 $SERVER_PID
	fi

	tst_rmdir
}

. tst_net.sh
do_setup

for i in $(seq 1 $TST_TOTAL); do
	do_test
done

tst_exit
