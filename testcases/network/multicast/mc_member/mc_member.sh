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
# TEST DESCRIPTION : To verify that two of the new options for level IPPROTO_IP
#                    Service Interface allow the list of host group memberships
#                    to be updated properly in response to the JoinHostGroup and
#                    LeaveHostGroup requests. To test boundary conditions while
#                    exercising IP Multicast JoinHostGroup and LeaveHostGroup
#                    Service Interfaces.
#
# HISTORY: 03/26/01 Robbie Williamson (robbiew@us.ibm.com) Ported

GLIST=${GLIST:-$LTPROOT/testcases/data/mc_member/ManyGroups}
TooManyGLIST=${TooManyGLIST:-$LTPROOT/testcases/data/mc_member/TooManyGroups}
ERRFILE=${ERRFILE:-errors}

TCID=mc_member
TST_TOTAL=1
TST_COUNT=1
TST_CLEANUP=do_cleanup
TST_USE_LEGACY_API=1

setup()
{
	tst_require_cmds netstat
	tst_tmpdir
}

get_address()
{
	DIGIT=`ps -ef | grep mc_member.sh | grep -v grep | wc -l`
	ADDRESS=$DIGIT.$DIGIT.$DIGIT.$DIGIT
}

do_test()
{
	tst_resm TINFO "doing test."
	local ipaddr=$(tst_ipaddr)
	COUNT=1
	while [ $COUNT -le 2 ]
	do
		# Run setsockopt test with bogus network
		get_address
		tst_resm TINFO "Running mc_member_e on $ADDRESS"
		mc_member_test -j -g $GLIST -s 30 -i $ADDRESS >/dev/null 2>&1

		# Run setsockopt/getsockopt test
		mc_member_test -g $GLIST -s 80 -i $ipaddr > $ERRFILE 2>&1 &

		# Join twice and leave once and see if the groups are still
		# joined on all specified interfaces.
		for agroup in `cat $GLIST`
		do
			tst_resm TINFO "Running (1st) member on $ipaddr"
			mc_member_test -j -g $GLIST -s 30 -i $ipaddr \
				       > $ERRFILE 2>&1 &
			sleep 5
			grep "cannot join group" $ERRFILE
			if [ $? -eq 0 ]; then
				tst_brkm TFAIL "MC group could NOT join "\
					      "$ipaddr"
			fi

			tst_resm TINFO "Running (2nd) member on $ipaddr"
			mc_member_test -g $GLIST -s 60 -i $ipaddr \
				       > $ERRFILE 2>&1 &
			sleep 5
			grep "cannot join group" $ERRFILE
			if [ $? -eq 0 ]; then
				tst_brkm TFAIL "MC group could NOT join "\
					      "$ipaddr"
			fi
		done

		# See if the groups are joined
		for agroup in `cat $GLIST`
		do
			netstat -gn | grep $agroup
			if [ $? -ne 0 ]; then
				tst_brkm TFAIL "$agroup NOT joined to $ipaddr"
			fi
		done

		tst_resm TINFO "Waiting 60 seconds! Do not interrupt!"
		sleep 60 # Make sure the first process has stopped

		for agroup in `cat $GLIST`
		do
			netstat -gn | grep $agroup
			if [ $? -ne 1 ]; then
				tst_brkm TFAIL "$agroup still joined on "\
					      "$ipaddr"
			fi
		done

		# Test the membership boundaries
		tst_resm TINFO "Running member on too many groups over "\
			       "$ipaddr"
		mc_member_test -j -g $TooManyGLIST -i $ipaddr \
			       > $ERRFILE 2>&1 &

		count=`grep 105 $ERRFILE | wc -l`
		if [ $count -gt 3 ]; then
			tst_brkm TFAIL "Could not join members!"
		fi

		COUNT=$(( $COUNT + 1 ))

	done

	tst_resm TPASS "Test Successful"
	tst_exit
}

do_cleanup()
{
	tst_rmdir
}

. tst_net.sh
setup
do_test
