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
# File :        ip_tests.sh
#
# Description:  Test basic functionality of ip command in route2 package
#
# Author:       Manoj Iyer, manjo@mail.utexas.edu
#
# History:      Feb 19 2003 - Created - Manoj Iyer.
#
#! /bin/sh


# Function:		init
#
# Description:	- Check if command ip is available.
#               - Check if command ifconfig is available.
#               - check if command awk is available.
#               - alias eth0 to eth0:1 with IP 10.1.1.12
#
# Return		- zero on success
#               - non zero on failure. return value from commands ($RC)
init()
{

	export RC=0					# Return code from commands.
	export TST_TOTAL=1			# total numner of tests in this file.
	export TCID="ip_tests  "		# this is the init function.
	export TST_COUNT=0			# init identifier,

	if [ -z $TMP ]
	then
		LTPTMP=/tmp
	else
		LTPTMP=$TMP
	fi

	# Initialize cleanup function.
	trap "cleanup" 0

	# Check to see if test harness functions are in the path.
	which tst_resm  &>$LTPTMP/tst_ip.err || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brkm TBROK NULL \
			"INIT: USCTEST commands not found, set PATH correctly."
		return $RC
	fi

	which awk  &>$LTPTMP/tst_ip.err || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brkm TBROK NULL \
			"INIT: command awk not found. Exiting test."
		return $RC
	fi

	which ip  &>$LTPTMP/tst_ip.err || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brkm TBROK NULL \
			"INIT: command ip not found. Exiting test."
		return $RC
	fi

	which ifconfig  &>$LTPTMP/tst_ip.err || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brkm TBROK NULL \
			"INIT: command awk not found. Exiting test."
		return $RC
	fi

	tst_resm TINFO "INIT: Inititalizing tests."

	# Aliasing eth0 to create private network.
	/sbin/ifconfig eth0:1 10.1.1.12 &>$LTPTMP/tst_ip.err || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brk TBROK "INIT: failed aliasing eth0:1 with IP 10.1.1.12"
		return $RC
	else
		/sbin/route add -host 10.1.1.12 dev eth0:1 &>$LTPTMP/tst_ip.err \
			|| RC=$?
		if [ $RC -ne 0 ]
		then
			tst_brk TBROK "INIT: failed adding route to 10.1.1.12"
			return $RC
		else
			tst_resm TINFO "INIT: added alias: `ifconfig eth0:1`"
		fi
	fi

	return $RC
}


# Function:		cleanup
#
# Description	- remove temporary files and directories.
#               - remove alias to eth0
#
# Return		- zero on success
#               - non zero on failure. return value from commands ($RC)
cleanup()
{
	TCID=dhcpd
	TST_COUNT=0
	RC=0


	/sbin/ifconfig eth0:1 &>$LTPTMP/tst_ip.err || RC=$?
	if [ $RC -eq 0 ]
	then
		/sbin/ifconfig eth0:1 down &>$LTPTMP/tst_ip.err
	fi

	rm -fr $LTPTMP/tst_ip.*
	return $RC
}


# Function:		test01
#
# Description	- Test basic functionality of ip command
#               - Test #1: ip link set DEVICE mtu MTU changes the device mtu
#                 size.
#               - execute the command and create output.
#               - create expected output
#               - compare expected output with actual output.
#
# Return		- zero on success
#               - non zero on failure. return value from commands ($RC)

test01()
{
	RC=0			# Return value from commands.
	TCID=ip01	    # Name of the test case.
	TST_COUNT=1		# Test number.

	tst_resm TINFO \
	 "Test #1: ip link set DEVICE mtu MTU changes the device mtu size"

	tst_resm TINFO "Test #1: changing mtu size of eth0:1 device."

	ip link set eth0:1 mtu 300 &>$LTPTMP/tst_ip.err
	if [ $RC -ne 0 ]
	then
		tst_res TFAIL $LTPTMP/tst_ip.err \
			"Test #1: ip command failed. Reason: "
		return $RC
	else
		MTUSZ=`ifconfig eth0:1 | grep -i MTU | awk '{print $5}'`
		if [ $MTUSZ == "MTU:300" ]
		then
			tst_resm TPASS "Test #1: changing mtu size success"
		else
			tst_resm FAIL NULL \
				"Test #1: MTU value not set to 300: ifconfig returned: $MTUSZ"
			return $RC
		fi
	fi
	return $RC
}


# Function:		test02
#
# Description	- Test basic functionality of ip command
#               - Test #1: ip link show DEVICE lists device attributes.
#               - execute the command and create output.
#               - create expected output
#               - compare expected output with actual output.
#
# Return		- zero on success
#               - non zero on failure. return value from commands ($RC)

test02()
{
	RC=0			# Return value from commands.
	TCID=ip01	    # Name of the test case.
	TST_COUNT=1		# Test number.

	tst_resm TINFO \
	 "Test #1: ip link set DEVICE mtu MTU changes the device mtu size"

	tst_resm TINFO "Test #1: changing mtu size of eth0:1 device."

	ip link show eth0 &>$LTPTMP/tst_ip.err
	if [ $RC -ne 0 ]
	then
		tst_res TFAIL $LTPTMP/tst_ip.err \
			"Test #1: ip command failed. Reason: "
		return $RC
	else
		=`ifconfig eth0 | grep -i MTU | awk '{print $5}'`
		if [ $RC == "MTU:300" ]
		then
			tst_resm TPASS "Test #1: changing mtu size success"
		else
			tst_brkm TBROK NULL \
				"Test #1: MTU != 300: ifconfig returned $RC"
			return $RC
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
if [ $RC -eq 666 ]
then
	exit 0
else
	if [ $RC -ne 0 ]
	then
		exit $RC
	fi
fi

test01 || RC=$?

exit $RC
