#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001
#
#  Author:	Jan 20 2004 Hubert Lin <linux02NOSPAAAM@tw.ibm.com>
#				       <hubertNOSPAAAM@symbio.com.tw>

TST_CNT=6
TST_SETUP="init"
TST_TESTFUNC="test"
TST_CLEANUP="cleanup"
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="iptables grep ping telnet"

. tst_test.sh

init()
{
	tst_res TINFO "INIT: Inititalizing tests."

	modprobe ip_tables
	if [ $? -ne 0 ]; then
		iptables -L > tst_iptables.out 2>&1
		if [ $? -ne 0 ]; then
			tst_brk TCONF "no iptables support in kernel."
		fi
	fi

	tst_res TINFO "INIT: Flushing all rules."
	iptables -F -t filter > tst_iptables.out 2>&1
	iptables -F -t nat > tst_iptables.out 2>&1
	iptables -F -t mangle > tst_iptables.out 2>&1
}

cleanup()
{
	lsmod | grep "ip_tables" > tst_iptables.out 2>&1
	if [ $? -eq 0 ]; then
		iptables -F -t filter > tst_iptables.out 2>&1
		iptables -F -t nat > tst_iptables.out 2>&1
		iptables -F -t mangle > tst_iptables.out 2>&1
		rmmod -v ipt_limit ipt_multiport ipt_LOG ipt_REJECT \
			 iptable_mangle iptable_nat ip_conntrack \
			 iptable_filter ip_tables nf_nat_ipv4 nf_nat \
			 nf_log_ipv4 nf_log_common nf_reject_ipv4 \
			 nf_conntrack_ipv4 nf_defrag_ipv4 nf_conntrack \
			 > tst_iptables.out 2>&1
	fi
}

test1()
{
	local chaincnt=0

	local cmd="iptables -L -t filter"
	tst_res TINFO "$cmd will list all rules in table filter."
	$cmd > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$cmd failed to list rules."
		cat tst_iptables.out
		return
	else
		chaincnt=$(grep -c Chain tst_iptables.out)
		if [ $chaincnt -lt 3 ]; then
			tst_res TFAIL "$cmd failed to list rules."
			cat tst_iptables.out
			return
		else
			tst_res TINFO "$cmd lists rules."
		fi
	fi

	local cmd="iptables -L -t nat"
	tst_res TINFO "$cmd will list all rules in table nat."
	$cmd > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$cmd failed to list rules."
		cat tst_iptables.out
		return
	else
		chaincnt=$(grep -c Chain tst_iptables.out)
		if [ $chaincnt -lt 3 ]; then
			tst_res TFAIL "$cmd failed to list rules."
			cat tst_iptables.out
			return
		else
			tst_res TINFO "$cmd lists rules."
		fi
	fi

	local cmd="iptables -L -t mangle"
	tst_res TINFO "$cmd will list all rules in table mangle."
	$cmd > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$cmd failed to list rules."
		cat tst_iptables.out
		return
	else
		chaincnt=$(grep -c Chain tst_iptables.out)
		if [ $chaincnt -lt 5 ]; then
			tst_res TFAIL "$cmd failed to list rules."
			cat tst_iptables.out
		else
			tst_res TINFO "$cmd lists rules."
		fi
	fi

	tst_res TPASS "iptables -L lists rules."
}

test2()
{
	tst_res TINFO "Use iptables to DROP packets from particular IP"
	tst_res TINFO "Rule to block icmp from 127.0.0.1"

	iptables -A INPUT -s 127.0.0.1 -p icmp -j DROP > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "iptables command failed to append new rule."
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Pinging 127.0.0.1"
	ping -c 2 127.0.0.1 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		grep "100% packet loss" tst_iptables.out > tst_iptables.err 2>&1
		if [ $? -ne 0 ]; then
			tst_res TFAIL \
				 "iptables did not block packets from loopback"
			cat tst_iptables.err
			return
		else
			tst_res TINFO "Ping 127.0.0.1 not successful."
		fi
	else
		tst_res TFAIL "iptables did not block icmp from 127.0.0.1"
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Deleting icmp DROP from 127.0.0.1 rule."
	iptables -D INPUT 1 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "iptables did not remove the rule."
		cat tst_iptables.out
		return
	fi
	tst_res TINFO "Pinging 127.0.0.1 again"
	ping -c 2 127.0.0.1 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "iptables blocking loopback. This is expected" \
			       "behaviour on certain distributions where" \
			       "enabling firewall drops all packets by default."
		cat tst_iptables.out
		return
	fi
	tst_res TINFO "Ping succsess"
	tst_res TPASS "iptables can DROP packets from particular IP."
}

test3()
{
	tst_res TINFO "Use iptables to REJECT ping request."
	tst_res TINFO "Rule to reject ping request."

	iptables -A INPUT -p icmp --icmp-type echo-request -d 127.0.0.1 -j \
		 REJECT > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "iptables command failed to append new rule."
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Pinging 127.0.0.1"
	ping -c 2 127.0.0.1 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		grep "100% packet loss" tst_iptables.out > tst_iptables.err 2>&1
		if [ $? -ne 0 ]; then
			tst_res TFAIL "iptables did not block ping request."
			cat tst_iptables.err
			return
		else
			tst_res TINFO "Ping 127.0.0.1 not successful."
		fi
	else
		tst_res TFAIL "iptables did not reject ping request."
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Deleting icmp request REJECT rule."
	iptables -D INPUT 1 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "iptables did not remove the rule."
		cat tst_iptables.out
		return
	fi
	tst_res TINFO "Pinging 127.0.0.1 again"
	ping -c 2 127.0.0.1 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "iptables blocking ping requests. This is" \
			      "expected behaviour on certain distributions" \
			      "where enabling firewall drops all packets by" \
			      "default."
		cat tst_iptables.out
		return
	fi
	tst_res TINFO "Ping succsess"
	tst_res TPASS "iptables can REJECT ping requests."
}

test4()
{
	local dport=45886
	local logprefix="${TCID}$(date +%m%d%H%M%S):"

	tst_res TINFO "Use iptables to log packets to particular port."
	tst_res TINFO "Rule to log tcp packets to particular port."

	iptables -A INPUT -p tcp -d 127.0.0.1 --dport $dport -j LOG \
		 --log-prefix "$logprefix" > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "iptables command failed to append new rule."
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "telnet 127.0.0.1 $dport"
	telnet 127.0.0.1 $dport > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		sleep 2
		dmesg | grep "$logprefix" > tst_iptables.err 2>&1
		if [ $? -ne 0 ]; then
			tst_res TFAIL \
				 "iptables did not log packets to port $dport"
			cat tst_iptables.err
			return
		else
			tst_res TINFO "Packets to port $dport logged."
		fi
	else
		tst_res TFAIL "telnet to 127.0.0.1 $dport should fail."
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Deleting the rule to log."
	iptables -D INPUT 1 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "iptables did not remove the rule."
		cat tst_iptables.out
		return
	fi
	tst_res TINFO "iptables logging succsess"
	tst_res TPASS "iptables can log packets to particular port."
}

test5()
{
	local dport=0
	local logprefix="${TCID}$(date +%m%d%H%M%S):"

	tst_res TINFO "Use iptables to log packets to multiple ports."
	tst_res TINFO "Rule to log tcp packets to port 45801 - 45803."
	iptables -A INPUT -p tcp -d 127.0.0.1 --dport 45801:45803 -j LOG \
		 --log-prefix "$logprefix" > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "iptables command failed to append new rule."
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Rule to log tcp packets to port 45804 - 45806."
	iptables -A INPUT -p tcp -d 127.0.0.1 -m multiport --dports \
		 45804,45806,45805 -j LOG --log-prefix "$logprefix" \
		 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "iptables command failed to append new rule."
		cat tst_iptables.out
		return
	fi

	for dport in 45801 45802 45803 45804 45805 45806; do
		tst_res TINFO "telnet 127.0.0.1 $dport"
		telnet 127.0.0.1 $dport > tst_iptables.out 2>&1
		if [ $? -ne 0 ]; then
			sleep 2
			dmesg | grep "$logprefix" | grep "=$dport " \
				> tst_iptables.err 2>&1
			if [ $? -ne 0 ]; then
				tst_res TFAIL "iptables did not log packets" \
					       "to port $dport"
				cat tst_iptables.err
				return
			else
				tst_res TINFO "Packets to port $dport logged."
			fi
		else
			tst_res TFAIL "telnet to 127.0.0.1 $dport should fail."
			cat tst_iptables.out
			return
		fi
	done

	tst_res TINFO "Flushing all rules."
	iptables -F > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "iptables did not flush all rules."
		cat tst_iptables.out
		return
	fi
	tst_res TINFO "iptables logging succsess"
	tst_res TPASS "iptables can log packets to multiple ports."
}

test6()
{
	local logcnt=0
	local logprefix="${TCID}$(date +%m%d%H%M%S):"

	tst_res TINFO "Use iptables to log ping request with limited rate."
	tst_res TINFO "Rule to log ping request."

	iptables -A INPUT -p icmp --icmp-type echo-request -d 127.0.0.1 -m \
		 limit -j LOG --log-prefix "$logprefix" > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "iptables command failed to append new rule."
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "ping 127.0.0.1"
	ping -c 10 127.0.0.1 > tst_iptables.out 2>&1
	if [ $? -eq 0 ]; then
		sleep 2
		logcnt=$(dmesg | grep -c "$logprefix")
		if [ $logcnt -ne 5 ]; then
			tst_res TFAIL "iptables did not log packets with" \
				      "limited rate."
			cat tst_iptables.out
			return
		else
			tst_res TINFO "ping requests logged with limited rate."
		fi
	else
		tst_res TFAIL "ping to 127.0.0.1 failed. This is expected" \
			      "behaviour on certain distributions where" \
			      "enabling firewall drops all packets by default."
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Deleting the rule to log."
	iptables -D INPUT 1 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "iptables did not remove the rule."
		cat tst_iptables.out
		return
	fi
	tst_res TINFO "iptables limited logging succsess"
	tst_res TPASS "iptables can log packets with limited rate."
}

tst_run
