#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2001                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :        dhcpd_tests.sh
#
# Description:  Test basic functionality of dhcpd server.
#				- Test #1:  dhcpd server starts up and follows rules in
#				            dhcp.conf file.
#
# Author:       Manoj Iyer, manjo@mail.utexas.edu
#
# History:      Feb 11 2003 - Created - Manoj Iyer.
#               Feb 12 2003 - Added - Manoj Iyer. Added test to start and stop
#                             dhcp server using rules in dhcp.conf file.
#               Feb 13 2003 - Added - Manoj Iyer. added check to see if command
#                             is installed.
#               Feb 28 2003 - Fixed testcase from incorrectly exiting test with
#                             a PASS result when dhcpd is not running.
#               Mar 02 2003 - Fixed exit code check, removed return 666.
#                             `ps -ef | grep dhcpd` was returning success even
#                             if dhcpd was not running. Worked around this
#                             problem.
#
# Function:		init
#
# Description:	- Check if command dhcpd is available.
#				- Check if /etc/dhcpd.conf is available.
#               - Create temporary config file, for dhcpd.
#               - alias ethX to ethX:1 with IP 10.1.1.12
#
# Return		- zero on success
#               - non zero on failure. return value from commands ($RC)
init()
{

	export RC=0					# Return code from commands.
	export TST_TOTAL=1			# total numner of tests in this file.
	export TCID="dhcpd  "		# this is the init function.
	export TST_COUNT=0			# init identifier,

	if [ -z $TMP ]
	then
		LTPTMP=/tmp/tst_dhcpd.$$/
	else
		LTPTMP=$TMP/tst_dhcpd.$$/
	fi

	# Initialize cleanup function.
	trap "cleanup" 0

	# create the temporary directory used by this testcase
	mkdir -p $LTPTMP/ > /dev/null 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brkm TBROK "INIT: Unable to create temporary directory"
		return $RC
	fi

	which tst_resm  > $LTPTMP/tst_dhcpd.err 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brkm TBROK NULL \
			"INIT: USCTEST commands not found, set PATH correctly."
		return $RC
	fi

	which awk  > $LTPTMP/tst_dhcpd.err 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brkm TBROK NULL \
			"INIT: command awk not found. Exiting test."
		return $RC
	fi

	tst_resm TINFO "INIT: Inititalizing tests."

	ps -ef > $LTPTMP/tst_dhcpd.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_res TBROK $LTPTMP/tst_dhcpd.out NULL \
			"INIT: ps command failed. Reason:"
		return $RC
	fi

	grep "dhcpd[[:blank:]]" $LTPTMP/tst_dhcpd.out > $LTPTMP/tst_dhcpd.err 2>&1 || RC=$?
	if [ $RC -eq 0 ]
	then
		tst_resm TPASS "INIT: dhcpd is already running. Declaring success"
		exit 0
	fi

	RC=0
	which dhcpd > $LTPTMP/tst_dhcpd.err 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brk TBROK $LTPTMP/tst_dhcpd.err NULL \
			"INIT: dhcpd command does not exist. Reason:"
		return $RC
	fi

	cat > $LTPTMP/tst_dhcpd.conf <<-EOF || RC=$?
	subnet 10.1.1.0 netmask 255.255.255.0 {
        # default gateway
		range 10.1.1.12 10.1.1.12;
		default-lease-time 600;
		max-lease-time 1200;
		option routers 10.1.1.1;
		option subnet-mask
		255.255.255.0;
		option
		domain-name-servers
		10.1.1.1;
		option
		domain-name
		"dhcptest.net";
	}
	ddns-update-style ad-hoc;
	EOF
	if [ $RC -ne 0 ]
	then
		tst_brkm TBROK NULL \
			"INIT: Unable to create temp file: $LTPTMP/tst_dhcpd.conf"
		return $RC
	fi

	if [ -f /etc/dhcpd.conf ]
	then
		RC1=0
		mv /etc/dhcpd.conf $LTPTMP/dhcpd.conf > $LTPTMP/tst_dhcpd.err 2>&1 || RC=$?
		mv $LTPTMP/tst_dhcpd.conf /etc/dhcpd.conf > $LTPTMP/tst_dhcpd.err 2>&1 \
			|| RC1=$?
		if [ $RC -ne 0 -o $RC1 -ne 0 ]
		then
			tst_brkm TBROK NULL \
				"INIT: Failed to create dhcpd.conf file."
			return $(($RC+1))
		fi
	else
		tst_brkm TBROK NULL \
			"INIT: No /etc/dhcpd.conf file found."
		return $(($RC+1))
	fi

  # Checking if exist a valid network interface
  /sbin/ifconfig | grep -q "^eth" || RC=$?
	if [ $RC -ne 0 ]
  then
		tst_brkm TBROK NULL "INIT: failed identifying a valid eth interface"
    return $RC
  fi

  # Get the eth interface
  ETH_INTERFACE=$(/sbin/ifconfig | grep -m 1 "^eth" | awk -F" " '{ print $1 }')

	# Aliasing eth to create private network.
	/sbin/ifconfig ${ETH_INTERFACE}:1 10.1.1.12 > $LTPTMP/tst_dhcpd.err 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brkm TBROK NULL "INIT: failed aliasing ${ETH_INTERFACE}:1 with IP 10.1.1.12"
		return $RC
	else
		/sbin/route add -host 10.1.1.12 dev ${ETH_INTERFACE}:1 > $LTPTMP/tst_dhcpd.err 2>&1 \
			|| RC=$?
		if [ $RC -ne 0 ]
		then
			tst_brkm TBROK NULL "INIT: failed adding route to 10.1.1.12"
			return $RC
		fi
	fi

	return $RC
}


# Function:		cleanup
#
# Description	- remove temporary files and directories.
#
# Return		- zero on success
#               - non zero on failure. return value from commands ($RC)
cleanup()
{
	TCID=dhcpd
	TST_COUNT=0
	RC=0

	if [ -f $LTPTMP/dhcpd.conf ]
	then
		mv $LTPTMP/dhcpd.conf /etc/dhcpd.conf > $LTPTMP/tst_dhcpd.err 2>&1
	fi

	/sbin/ifconfig | grep "${ETH_INTERFACE}:1" > $LTPTMP/tst_dhcpd.err 2>&1 || RC=$?
	if [ $RC -eq 0 ]
	then
		/sbin/ifconfig ${ETH_INTERFACE}:1 down > $LTPTMP/tst_dhcpd.err 2>&1
	fi

  # Removing the added route
  /sbin/route del 10.1.1.12 

	rm -fr $LTPTMP
	return $RC
}


# Function:		test01
#
# Description	- Test basic functionality of dhcpd.
#               - Test #1: dhcpd will serve IP addresses based on rules in 
#                 /etc/dhcpd.conf file.
#				- create dhcpd.conf file, server to listen to ethX/.../10.1.1.0
#               - start dhcpd server
#               - create expected output
#               - compare expected output with actual output.
#
# Return		- zero on success
#               - non zero on failure. return value from commands ($RC)

test01()
{
	RC=0			# Return value from commands.
	TCID=dhcpd01	# Name of the test case.
	TST_COUNT=1		# Test number.

	tst_resm TINFO \
	 "Test #1: dhcpd will serve IPaddr, rules in /etc/dhcpd.conf file."

	hwaddr=`ifconfig ${ETH_INTERFACE} | grep HWaddr | awk '{print $5}'` || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brkm TBROK NULL "Test #1: Failed to get hardware address."
		return $RC
	fi

	cat > $LTPTMP/tst_dhcpd.exp <<-EOF || RC=$?
	Sending on   Socket/fallback/fallback-net
	EOF
	if [ $RC -ne 0 ]
	then
		tst_brkm TBROK NULL "Test #1: Unable to create expected reaults."
		return $RC
	fi
	
	tst_resm TINFO "Test #1: starting dhcp server"
	dhcpd > $LTPTMP/tst_dhcpd.err 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_res TFAIL $LTPTMP/tst_dhcpd.err \
			"Test #1: Failed to start dhcp. Reason: "
		return $RC
	else
		cat $LTPTMP/tst_dhcpd.err | tail -n 1 > $LTPTMP/tst_dhcpd.out 2>&1 || RC=$?
		if [ $RC -ne 0 ]
		then
			tst_brkm TBROK NULL \
				"Test #1: unable to create output file."
			return $RC
		fi
	fi

	diff -iwB $LTPTMP/tst_dhcpd.out $LTPTMP/tst_dhcpd.exp \
		> $LTPTMP/tst_dhcpd.err 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_res TFAIL $LTPTMP/tst_dhcpd.err \
			"Test #1: failed starting dhcpd server. Reason:"
		return $RC
	else
		tst_res TINFO $LTPTMP/tst_dhcpd.out "Test #1 DHCP server statistics:"
		tst_resm TINFO "Test #1: stoping dhcp server"
		killall dhcpd > $LTPTMP/tst_dhcpd.err 2>&1 || RC=$?
		if [ $RC -ne 0 ]
		then
			tst_brk TBROK $LTPTMP/tst_dhcpd.err NULL \
				"Test #1: Failed to stop dhcp server. Reason:"
			return $RC
		else
			tst_resm TPASS "Test #1: dhcpd started and stoped successfully"
		fi
	fi
	return $RC
}


# Function:		main
#
# Description:	- Execute all tests, report results.
#               
# Exit:			- zero on success
# 				- non-zero on failure.
TFAILCNT=0			# Set TFAILCNT to 0, increment on failure.
RC=0				# Return code from test.

init || RC=$?
if [ $RC -ne 0 ]
then
	exit $RC
fi

test01 || RC=$?

exit $RC
