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
# File :        ipchains_tests.sh
#
# Description:  Test basic functionality of ipchains (firewall administration)
#				- Test #1:  ipchains -L will list all rules in the selected
#               - Test #2:  Test ipchains deny packets from perticular IP.
#				chain.
#               
# Author:       Manoj Iyer, manjo@mail.utexas.edu
#
# History:      Feb 10 2003 - Created - Manoj Iyer.
#
#! /bin/sh


# Function:		init
#
# Description:	- Check if command ipchains is available.
# Description:	- Check if ipchains kernel support is available.
#               - Initialize environment variables.
#
# Return		- zero on success
#               - non zero on failure. return value from commands ($RC)
init()
{

	export RC=0					# Return code from commands.
	export TST_TOTAL=1			# total numner of tests in this file.
	export TCID="ipchains  "	# this is the init function.
	export TST_COUNT=0			# init identifier,

	if [ -z $TMP ]
	then
		LTPTMP=/tmp
	else
		LTPTMP=$TMP
	fi

	# Initialize cleanup function.
	trap "cleanup" 0

	which tst_resm  &>$LTPTMP/tst_ipchains.err || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brkm TBROK \
			"Test INIT: USCTEST commands not found, set PATH correctly."
		return $RC
	fi

	tst_resm TINFO "INIT: Inititalizing tests."
	which ipchains &> $LTPTMP/tst_ipchains.err || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brk TBROK $LTPTMP/tst_ipchains.err NULL \
			"Test INIT: ipchains command does not exist. Reason:"
		return $RC
	fi

	modprobe ipchains &>$LTPTMP/tst_ipchains.err || RC=$?
	if [ $RC -ne 0 ]
	then
		RC=0
		ipchains -L &>$LTPTMP/tst_ipchains.err || RC=$?
		if [ $RC -ne 0 ]
		then
			tst_brk TBROK $LTPTMP/tst_ipchains.err NULL \
				"Test INIT: no ipchains support in kenrel. Reason:"
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
	TCID=ipchains
	TST_COUNT=0
	RC=0

	lsmod | grep "ipchains" &>$LTPTMP/tst_ipchains.err || RC=0
	if [ $RC -eq 0 ]
	then
		rmmod ipchains &>$LTPTMP/tst_ipchains.err 
	fi

	rm -fr $LTPTMP/tst_ipchains.*
	return $RC
}


# Function:		test01
#
# Description	- Test basic functionality of ipchains (firewall administration)
#               - Test #1:  ipchains -L will list all rules in the selected
#                 chain.
#               - create expected output
#               - compare expected output with actual output.
#
# Return		- zero on success
#               - non zero on failure. return value from commands ($RC)

test01()
{
	RC=0			# Return value from commands.
	TCID=ipchains01	# Name of the test case.
	TST_COUNT=1		# Test number.

	tst_resm TINFO \
		"Test #1: ipchains -L will list all rules in selected chain."

	cat > $LTPTMP/tst_ipchains.exp <<-EOF || RC=$?
	Chain input (policy ACCEPT):
	Chain forward (policy ACCEPT):
	Chain output (policy ACCEPT):
	EOF
	if [ $RC -ne 0 ]
	then
		tst_brkm TBROK "Test #1: Unable to create expected reaults."
		return $RC
	fi

	ipchains -L &>$LTPTMP/tst_ipchains.out || RC=$?
    if [ $RC -ne 0 ]
	then
		tst_res TFAIL $LTPTMP/tst_ipchains.out \
			"Test #1: ipchains -L failed to list rules. Reason:"
		return $RC
	else
		diff $LTPTMP/tst_ipchains.out $LTPTMP/tst_ipchains.exp \
			&>$LTPTMP/tst_ipchains.err || RC=$?
		if [ $RC -ne 0 ]
		then
			tst_res TFAIL $LTPTMP/tst_ipchains.err \
				"Test #1: ipchains -L failed to list rules. Reason:"
			return $RC
		else
			tst_resm TPASS "Test #1: ipchains -L lists rules."
		fi
	fi

	return $RC
}


# Function:		test02
#
# Description	- Test basic functionality of ipchains (firewall administration)
#               - Test #2:  Test ipchains deny packets from perticular IP.
#               - Append new rule to block all packets from loopback.
#				- ping -c 2 loopback, this should fail. 
#				- remove rule, and ping -c loopback, this should work.
#
# Return		- zero on success
#               - non zero on failure. return value from commands ($RC)
test02()
{
	RC=0			# Return value from commands.
	TCID=ipchains02	# Name of the test case.
	TST_COUNT=2		# Test number.

	tst_resm TINFO \
		"Test #2: Use ipchains to deny packets from perticular IP"
	tst_resm TINFO "Test #2: Rule to block icmp from 127.0.0.1"

	ipchains -A input -s 127.0.0.1 -p icmp -j DENY \
		&>$LTPTMP/tst_ipchains.err || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brk TBROK $LTPTMP/tst_ipchains.err NULL \
			"Test #2: ipchains command failed to append new rule. Reason:"
		return $RC
	fi

	tst_resm TINFO "Test #2: Pinging 127.0.0.1"
	ping -c 2 127.0.0.1 &>$LTPTMP/tst_ipchains.out || RC=$?
	if [ $RC -ne 0 ]
	then
		RC=0
		grep "100% packet loss" $LTPTMP/tst_ipchains.out \
			&>$LTPTMP/tst_ipchains.err || RC=$?
		if [ $RC -ne 0 ]
		then
			tst_res TFAIL $LTPTMP/tst_ipchains.err \
				"Test #2: ipchains did not block packets from loopback"
			return $RC
		else
			tst_resm TINFO "Test #2: Ping 127.0.0.1 not successful."
		fi
	else
		tst_res TFAIL $LTPTMP/tst_ipchains.out \
			"Test #2: ipchains did not block icmp from 127.0.0.1"
	fi

	tst_resm TINFO "Test #2: Deleting icmp DENY from 127.0.0.1 rule."
	ipchains -D input 1 &>$LTPTMP/tst_ipchains.out || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_res TFAIL $LTPTMP/tst_ipchains.err \
			"Test #2: ipchains did not remove the rule. Reason:"
		return $RC
	fi
	tst_resm TINFO "Test #2: Pinging 127.0.0.1 again"
	ping -c 2 127.0.0.1 &>$LTPTMP/tst_ipchains.out || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_res TFAIL $LTPTMP/tst_ipchains.err \
			"Test #2: ipchains blocking loopback. Reason:"
		return $RC
	else
		tst_resm TINFO "Test #2: Ping succsess"
		tst_resm TPASS "Test #2: ipchains can deny packets from perticular IP."
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

init || exit $RC # Exit if initializing testcases fails.	

test01 || RC=$?
if [ $RC -ne 0 ]
then
	TFAILCNT=$(($TFAILCNT+1))
fi

RC=0				# Return code from test.
test02 || RC=$?
if [ $RC -ne 0 ]
then
	TFAILCNT=$(($TFAILCNT+1))
fi


exit $TFAILCNT
