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
# File :        iptables_tests.sh
#
# Description:  Test basic functionality of iptables (firewall administration)
#		- Test #1:  iptables -L -t <table> will list all rules in the 
#		  selected table.
#		- Test #2:  Test iptables DROP packets from particular IP.
#		- Test #3:  Test iptables REJECT ping request.
#		- Test #4:  Test iptables log packets to single port.
#		- Test #5:  Test iptables log packets to multiple ports.
#		- Test #6:  Test limit matched logging for ping request.
#               
#
# History: 
#		Jan 20 2004 Hubert Lin <linux02NOSPAAAM@tw.ibm.com or hubertNOSPAAAM@symbio.com.tw>
#		  - Ported test01, test02 from Manoj Iyer's ipchains_tests.sh
#		  - Added test03, test04, test05, test06
#
# Function:	init
#
# Description:	- Check if command iptables is available.
# Description:	- Check if iptables kernel support is available.
#               - Initialize environment variables.
#
# Return	- zero on success
#               - non zero on failure. return value from commands ($RC)
init()
{

	export RC=0			# Return code from commands.
	export TST_TOTAL=6		# total numner of tests in this file.
	export TCID="iptables"		# Test case identifier
	export TST_COUNT=0		# init identifier

	if [ -z $TMP ]; then
		LTPTMP=/tmp
	else
		LTPTMP=$TMP
	fi

	# Initialize cleanup function.
	trap "cleanup" 0

	which tst_resm  > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_brkm TBROK \
			"Test INIT: USCTEST commands not found, set PATH correctly."
		return $RC
	fi

	tst_resm TINFO "INIT: Inititalizing tests."
	which iptables > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_brk TBROK $LTPTMP/tst_iptables.out NULL \
			"Test INIT: iptables command does not exist. Reason:"
		return $RC
	fi

	modprobe ip_tables > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		RC=0
		iptables -L > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
		if [ $RC -ne 0 ]; then
			tst_brk TBROK $LTPTMP/tst_iptables.out NULL \
				"Test INIT: no iptables support in kenrel. Reason:"
			return $RC
		fi
	fi

	tst_resm TINFO "INIT: Flushing all rules."
	iptables -F -t filter > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	iptables -F -t nat > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	iptables -F -t mangle > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	return $RC
}


# Function:	cleanup
#
# Description	- remove temporary files and directories.
#
# Return	- zero on success
#               - non zero on failure. return value from commands ($RC)
cleanup()
{
	TCID=iptables
	TST_COUNT=0
	RC=0

	lsmod | grep "ip_tables" > $LTPTMP/tst_iptables.out 2>&1 || RC=0
	if [ $RC -eq 0 ]; then
		iptables -F -t filter > $LTPTMP/tst_iptables.out 2>&1
		iptables -F -t nat > $LTPTMP/tst_iptables.out 2>&1
		iptables -F -t mangle > $LTPTMP/tst_iptables.out 2>&1
		rmmod -v ipt_limit ipt_multiport ipt_LOG ipt_REJECT iptable_mangle iptable_nat ip_conntrack iptable_filter ip_tables > $LTPTMP/tst_iptables.out 2>&1 
	fi
	rm -fr $LTPTMP/tst_iptables.*
	return $RC
}


# Function:	test01
#
# Description	- Test basic functionality of iptables (firewall administration)
#               - Test #1:  iptables -L -t <table> will list all rules in the 
#                 selected table.
#
# Return	- zero on success
#               - non zero on failure. return value from commands ($RC)

test01()
{
	RC=0			# Return value from commands.
	TCID=iptables01		# Name of the test case.
	TST_COUNT=1		# Test number.

	local chaincnt=0	# chain counter

	local cmd="iptables -L -t filter"
	tst_resm TINFO \
		"$TCID: $cmd will list all rules in table filter."
	$cmd > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_res TFAIL $LTPTMP/tst_iptables.out \
			"$TCID: $cmd failed to list rules. Reason:"
		return $RC
	else
		chaincnt=$(grep -c Chain $LTPTMP/tst_iptables.out)
		if [ $chaincnt -lt 3 ]; then
			tst_res TFAIL $LTPTMP/tst_iptables.out \
				"$TCID: $cmd failed to list rules. Reason:"
			return $chaincnt
		else
			tst_resm TINFO "$TCID: $cmd lists rules."
		fi
	fi

	local cmd="iptables -L -t nat"
	tst_resm TINFO \
		"$TCID: $cmd will list all rules in table nat."
	$cmd > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_res TFAIL $LTPTMP/tst_iptables.out \
			"$TCID: $cmd failed to list rules. Reason:"
		return $RC
	else
		chaincnt=$(grep -c Chain $LTPTMP/tst_iptables.out)
		if [ $chaincnt -ne 3 ]; then
			tst_res TFAIL $LTPTMP/tst_iptables.out \
				"$TCID: $cmd failed to list rules. Reason:"
			return $chaincnt
		else
			tst_resm TINFO "$TCID: $cmd lists rules."
		fi
	fi

	local cmd="iptables -L -t mangle" 
	tst_resm TINFO \
		"$TCID: $cmd will list all rules in table mangle."
	$cmd > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_res TFAIL $LTPTMP/tst_iptables.out \
			"$TCID: $cmd failed to list rules. Reason:"
		return $RC
	else
		chaincnt=$(grep -c Chain $LTPTMP/tst_iptables.out)
		if [ $chaincnt -ne 5 ]; then
			tst_res TFAIL $LTPTMP/tst_iptables.out \
				"$TCID: $cmd failed to list rules. Reason:"
			return $chaincnt
		else
			tst_resm TINFO "$TCID: $cmd lists rules."
		fi
	fi

	tst_resm TPASS "$TCID: iptables -L lists rules."
	return $RC
}


# Function:	test02
#
# Description	- Test basic functionality of iptables (firewall administration)
#               - Test #2:  Test iptables DROP packets from particular IP.
#               - Append new rule to block all packets from loopback.
#				- ping -c 2 loopback, this should fail. 
#				- remove rule, and ping -c loopback, this should work.
#
# Return	- zero on success
#               - non zero on failure. return value from commands ($RC)
test02()
{
	RC=0			# Return value from commands.
	TCID=iptables02		# Name of the test case.
	TST_COUNT=2		# Test number.

	tst_resm TINFO \
		"$TCID: Use iptables to DROP packets from particular IP"
	tst_resm TINFO "$TCID: Rule to block icmp from 127.0.0.1"

	iptables -A INPUT -s 127.0.0.1 -p icmp -j DROP \
		> $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_brk TBROK $LTPTMP/tst_iptables.out NULL \
			"$TCID: iptables command failed to append new rule. Reason:"
		return $RC
	fi

	tst_resm TINFO "$TCID: Pinging 127.0.0.1"
	ping -c 2 127.0.0.1 > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		RC=0
		grep "100% packet loss" $LTPTMP/tst_iptables.out \
			> $LTPTMP/tst_iptables.err 2>&1 || RC=$?
		if [ $RC -ne 0 ]; then
			tst_res TFAIL $LTPTMP/tst_iptables.out \
				"$TCID: iptables did not block packets from loopback"
			return $RC
		else
			tst_resm TINFO "$TCID: Ping 127.0.0.1 not successful."
		fi
	else
		tst_res TFAIL $LTPTMP/tst_iptables.out \
			"$TCID: iptables did not block icmp from 127.0.0.1"
		return $RC
	fi

	tst_resm TINFO "$TCID: Deleting icmp DROP from 127.0.0.1 rule."
	iptables -D INPUT 1 > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_res TFAIL $LTPTMP/tst_iptables.out \
			"$TCID: iptables did not remove the rule. Reason:"
		return $RC
	fi
	tst_resm TINFO "$TCID: Pinging 127.0.0.1 again"
	ping -c 2 127.0.0.1 > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_res TFAIL $LTPTMP/tst_iptables.out \
			"$TCID: iptables blocking loopback. This is expected behaviour on certain distributions where enabling firewall drops all packets by default."
		return $RC
	else
		tst_resm TINFO "$TCID: Ping succsess"
		tst_resm TPASS "$TCID: iptables can DROP packets from particular IP."
	fi
	
	return $RC
}


# Function:	test03
#
# Description	- Test basic functionality of iptables (firewall administration)
#               - Test #3:  Test iptables REJECT ping request.
#               - Append new rule to block all packets from loopback.
#				- ping -c 2 loopback, this should fail. 
#				- remove rule, and ping -c loopback, this should work.
#
# Return	- zero on success
#               - non zero on failure. return value from commands ($RC)
test03()
{
	RC=0			# Return value from commands.
	TCID=iptables03		# Name of the test case.
	TST_COUNT=3		# Test number.

	tst_resm TINFO \
		"$TCID: Use iptables to REJECT ping request."
	tst_resm TINFO "$TCID: Rule to reject ping request."

	iptables -A INPUT -p icmp --icmp-type echo-request -d 127.0.0.1 -j REJECT \
		> $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_brk TBROK $LTPTMP/tst_iptables.out NULL \
			"$TCID: iptables command failed to append new rule. Reason:"
		return $RC
	fi

	tst_resm TINFO "$TCID: Pinging 127.0.0.1"
	ping -c 2 127.0.0.1 > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		RC=0
		grep "100% packet loss" $LTPTMP/tst_iptables.out \
			> $LTPTMP/tst_iptables.err 2>&1 || RC=$?
		if [ $RC -ne 0 ]; then
			tst_res TFAIL $LTPTMP/tst_iptables.out \
				"$TCID: iptables did not block ping request."
			return $RC
		else
			tst_resm TINFO "$TCID: Ping 127.0.0.1 not successful."
		fi
	else
		tst_res TFAIL $LTPTMP/tst_iptables.out \
			"$TCID: iptables did not reject ping request."
		return $RC
	fi

	tst_resm TINFO "$TCID: Deleting icmp request REJECT rule."
	iptables -D INPUT 1 > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_res TFAIL $LTPTMP/tst_iptables.out \
			"$TCID: iptables did not remove the rule. Reason:"
		return $RC
	fi
	tst_resm TINFO "$TCID: Pinging 127.0.0.1 again"
	ping -c 2 127.0.0.1 > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_res TFAIL $LTPTMP/tst_iptables.out \
			"$TCID: iptables blocking ping requests. This is expected behaviour on certain distributions where enabling firewall drops all packets by default."
		return $RC
	else
		tst_resm TINFO "$TCID: Ping succsess"
		tst_resm TPASS "$TCID: iptables can REJECT ping requests."
	fi
	
	return $RC
}


# Function:	test04
#
# Description	- Test basic functionality of iptables (firewall administration)
#               - Test #4:  Test iptables log packets to single port
#               - Append new rule to log tcp packets to localhost:45886
#		- telnet localhost 45886, this should be logged.
#		- remove rule.
#
# Return	- zero on success
#               - non zero on failure. return value from commands ($RC)
test04()
{
	RC=0			# Return value from commands.
	TCID=iptables04		# Name of the test case.
	TST_COUNT=4		# Test number.
	local dport=45886				# destination port
	local logprefix="$TCID-$(date +%m%d%H%M%S):"	# log-prefix used by iptables

	tst_resm TINFO \
		"$TCID: Use iptables to log packets to particular port."
	tst_resm TINFO "$TCID: Rule to log tcp packets to particular port."

	iptables -A INPUT -p tcp -d 127.0.0.1 --dport $dport -j LOG --log-prefix "$logprefix" \
		> $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_brk TBROK $LTPTMP/tst_iptables.out NULL \
			"$TCID: iptables command failed to append new rule. Reason:"
		return $RC
	fi

	tst_resm TINFO "$TCID: telnet 127.0.0.1 $dport"
	telnet 127.0.0.1 $dport > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		RC=0
		sleep 2
		dmesg | grep "$logprefix" \
			> $LTPTMP/tst_iptables.err 2>&1 || RC=$?
		if [ $RC -ne 0 ]; then
			tst_res TFAIL $LTPTMP/tst_iptables.out \
				"$TCID: iptables did not log packets to port $dport"
			return $RC
		else
			tst_resm TINFO "$TCID: Packets to port $dport logged."
		fi
	else
		tst_res TFAIL $LTPTMP/tst_iptables.out \
			"$TCID: telnet to 127.0.0.1 $dport should fail."
		return $RC
	fi

	tst_resm TINFO "$TCID: Deleting the rule to log."
	iptables -D INPUT 1 > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_res TFAIL $LTPTMP/tst_iptables.out \
			"$TCID: iptables did not remove the rule. Reason:"
		return $RC
	else
		tst_resm TINFO "$TCID: iptables logging succsess"
		tst_resm TPASS "$TCID: iptables can log packets to particular port."
	fi

	return $RC
}


# Function:	test05
#
# Description	- Test basic functionality of iptables (firewall administration)
#               - Test #5:  Test iptables log packets to multiple ports
#               - Append new rule to log tcp packets to localhost port 45801 - 45803
#               - Append new rule to log tcp packets to localhost port 45804 - 45806 (ipt_multiport introduced)
#		- telnet localhost port 45801 - 45806, this should be logged.
#		- flush rules.
#
# Return	- zero on success
#               - non zero on failure. return value from commands ($RC)
test05()
{
	RC=0			# Return value from commands.
	TCID=iptables05		# Name of the test case.
	TST_COUNT=5		# Test number.
	local dport=0					# destination port
	local logprefix="$TCID-$(date +%m%d%H%M%S):"	# log-prefix used by iptables

	tst_resm TINFO \
		"$TCID: Use iptables to log packets to multiple ports."
	tst_resm TINFO "$TCID: Rule to log tcp packets to port 45801 - 45803."
	iptables -A INPUT -p tcp -d 127.0.0.1 --dport 45801:45803 -j LOG --log-prefix "$logprefix" \
		> $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_brk TBROK $LTPTMP/tst_iptables.out NULL \
			"$TCID: iptables command failed to append new rule. Reason:"
		return $RC
	fi

	tst_resm TINFO "$TCID: Rule to log tcp packets to port 45804 - 45806."
	iptables -A INPUT -p tcp -d 127.0.0.1 -m multiport --dports 45804,45806,45805 -j LOG --log-prefix "$logprefix" \
		> $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_brk TBROK $LTPTMP/tst_iptables.out NULL \
			"$TCID: iptables command failed to append new rule. Reason:"
		return $RC
	fi

	for dport in 45801 45802 45803 45804 45805 45806; do
		tst_resm TINFO "$TCID: telnet 127.0.0.1 $dport"
		telnet 127.0.0.1 $dport > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
		if [ $RC -ne 0 ]; then
			RC=0
			sleep 2
			dmesg | grep "$logprefix" | grep "=$dport " \
				> $LTPTMP/tst_iptables.err 2>&1 || RC=$?
			if [ $RC -ne 0 ]; then
				tst_res TFAIL $LTPTMP/tst_iptables.out \
					"$TCID: iptables did not log packets to port $dport"
				return $RC
			else
				tst_resm TINFO "$TCID: Packets to port $dport logged."
			fi
		else
			tst_res TFAIL $LTPTMP/tst_iptables.out \
				"$TCID: telnet to 127.0.0.1 $dport should fail."
			return $RC
		fi
	done

	tst_resm TINFO "$TCID: Flushing all rules."
	iptables -F > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_res TFAIL $LTPTMP/tst_iptables.out \
			"$TCID: iptables did not flush all rules. Reason:"
		return $RC
	else
		tst_resm TINFO "$TCID: iptables logging succsess"
		tst_resm TPASS "$TCID: iptables can log packets to multiple ports."
	fi

	return $RC
}


# Function:	test06
#
# Description	- Test basic functionality of iptables (firewall administration)
#               - Test #6:  Test limit matched logging for ping request.
#               - Append new rule to log ping request with rate of 3/hour, burst 5
#		- ping localhost 10 times, only the first 5 will be logged.
#		- remove rule.
#
# Return	- zero on success
#               - non zero on failure. return value from commands ($RC)
test06()
{
	RC=0			# Return value from commands.
	TCID=iptables06		# Name of the test case.
	TST_COUNT=6		# Test number.
	local logcnt=0		# log counter
	local logprefix="$TCID-$(date +%m%d%H%M%S):"	# log-prefix used by iptables

	tst_resm TINFO \
		"$TCID: Use iptables to log ping request with limited rate."
	tst_resm TINFO "$TCID: Rule to log ping request."

	iptables -A INPUT -p icmp --icmp-type echo-request -d 127.0.0.1 -m limit -j LOG --log-prefix "$logprefix" \
		> $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_brk TBROK $LTPTMP/tst_iptables.out NULL \
			"$TCID: iptables command failed to append new rule. Reason:"
		return $RC
	fi

	tst_resm TINFO "$TCID: ping 127.0.0.1"
	ping -c 10 127.0.0.1 > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -eq 0 ]; then
		RC=0
		sleep 2
		logcnt=$(dmesg | grep -c "$logprefix")
		if [ $logcnt -ne 5 ]; then
			tst_res TFAIL $LTPTMP/tst_iptables.out \
				"$TCID: iptables did not log packets with limited rate."
			return $logcnt
		else
			tst_resm TINFO "$TCID: ping requests logged with limited rate."
		fi
	else
		tst_res TFAIL $LTPTMP/tst_iptables.out \
			"$TCID: ping to 127.0.0.1 failed. This is expected behaviour on certain distributions where enabling firewall drops all packets by default."
		return $RC
	fi

	tst_resm TINFO "$TCID: Deleting the rule to log."
	iptables -D INPUT 1 > $LTPTMP/tst_iptables.out 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_res TFAIL $LTPTMP/tst_iptables.out \
			"$TCID: iptables did not remove the rule. Reason:"
		return $RC
	else
		tst_resm TINFO "$TCID: iptables limited logging succsess"
		tst_resm TPASS "$TCID: iptables can log packets with limited rate."
	fi

	return $RC
}

# Function:	main
#
# Description:	- Execute all tests, report results.
#              
# Exit:		- zero on success
# 		- non-zero on failure.
TFAILCNT=0	# Set TFAILCNT to 0, increment on failure.
RC=0		# Return code from test.

init || exit $RC # Exit if initializing testcases fails.	

test01 || RC=$?
if [ $RC -ne 0 ]; then
	TFAILCNT=$(($TFAILCNT+1))
fi

RC=0		# Return code from test.
test02 || RC=$?
if [ $RC -ne 0 ]; then
	TFAILCNT=$(($TFAILCNT+1))
fi

RC=0		# Return code from test.
test03 || RC=$?
if [ $RC -ne 0 ]; then
	TFAILCNT=$(($TFAILCNT+1))
fi

RC=0		# Return code from test.
test04 || RC=$?
if [ $RC -ne 0 ]; then
	TFAILCNT=$(($TFAILCNT+1))
fi

RC=0		# Return code from test.
test05 || RC=$?
if [ $RC -ne 0 ]; then
	TFAILCNT=$(($TFAILCNT+1))
fi

RC=0		# Return code from test.
test06 || RC=$?
if [ $RC -ne 0 ]; then
	TFAILCNT=$(($TFAILCNT+1))
fi

exit $TFAILCNT
