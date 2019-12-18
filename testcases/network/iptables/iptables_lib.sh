#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018-2019 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001
#
#  Author:	Jan 20 2004 Hubert Lin <linux02NOSPAAAM@tw.ibm.com>
#				       <hubertNOSPAAAM@symbio.com.tw>

TST_CNT=6
TST_TESTFUNC="test"
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_SETUP="${TST_SETUP:-init}"
TST_CLEANUP="${TST_CLEANUP:-cleanup}"

if [ "$use_iptables" = 1 ]; then
	toolname=iptables
	cmds="$toolname"
	TST_NEEDS_DRIVERS="ip_tables"
else
	toolname=nft
	cmds="$toolname iptables-translate"
	TST_NEEDS_DRIVERS="nf_tables"
fi

TST_NEEDS_CMDS="$cmds grep ping telnet"

. tst_test.sh

NFRUN()
{
	local rule

	if [ "$use_iptables" = 1 ]; then
		iptables $@
	else
		$(iptables-translate $@ | sed 's,\\,,g')
	fi
}

NFRUN_REMOVE()
{
	if [ "$use_iptables" = 1 ]; then
		ROD iptables -D INPUT 1
	else
		ROD nft flush chain ip filter INPUT
	fi
}

init()
{
	tst_res TINFO "INIT: Flushing all rules"
	NFRUN -F -t filter > tst_iptables.out 2>&1
	NFRUN -F -t nat > tst_iptables.out 2>&1
	NFRUN -F -t mangle > tst_iptables.out 2>&1
}

cleanup()
{
	if lsmod | grep -q "ip_tables"; then
		NFTRUN -F -t filter > /dev/null 2>&1
		NFTRUN -F -t nat > /dev/null 2>&1
		NFTRUN -F -t mangle > /dev/null 2>&1
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

	if [ "$use_iptables" != 1 ]; then
		tst_res TCONF "$toolname not applicable for test $1"
		return
	fi
	local chaincnt=0

	local cmd="iptables -L -t filter"
	tst_res TINFO "$cmd will list all rules in table filter"
	$cmd > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$cmd failed to list rules"
		cat tst_iptables.out
		return
	else
		chaincnt=$(grep -c Chain tst_iptables.out)
		if [ $chaincnt -lt 3 ]; then
			tst_res TFAIL "$cmd failed to list rules"
			cat tst_iptables.out
			return
		else
			tst_res TINFO "$cmd lists rules"
		fi
	fi

	local cmd="iptables -L -t nat"
	tst_res TINFO "$cmd will list all rules in table nat"
	$cmd > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$cmd failed to list rules"
		cat tst_iptables.out
		return
	else
		chaincnt=$(grep -c Chain tst_iptables.out)
		if [ $chaincnt -lt 3 ]; then
			tst_res TFAIL "$cmd failed to list rules"
			cat tst_iptables.out
			return
		else
			tst_res TINFO "$cmd lists rules"
		fi
	fi

	local cmd="iptables -L -t mangle"
	tst_res TINFO "$cmd will list all rules in table mangle"
	$cmd > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$cmd failed to list rules"
		cat tst_iptables.out
		return
	else
		chaincnt=$(grep -c Chain tst_iptables.out)
		if [ $chaincnt -lt 5 ]; then
			tst_res TFAIL "$cmd failed to list rules"
			cat tst_iptables.out
		else
			tst_res TINFO "$cmd lists rules"
		fi
	fi

	tst_res TPASS "iptables -L lists rules"
}

test2()
{
	tst_res TINFO "Use $toolname to DROP packets from particular IP"
	tst_res TINFO "Rule to block icmp from 127.0.0.1"

	NFRUN -A INPUT -s 127.0.0.1 -p icmp -j DROP > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$toolname command failed to append new rule"
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Pinging 127.0.0.1"
	ping -c 2 127.0.0.1 -W 1 -i 0 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		grep "100% packet loss" tst_iptables.out > tst_iptables.err 2>&1
		if [ $? -ne 0 ]; then
			tst_res TFAIL \
				 "$toolname did not block packets from loopback"
			cat tst_iptables.err
			return
		else
			tst_res TINFO "Ping 127.0.0.1 not successful"
		fi
	else
		tst_res TFAIL "$toolname did not block icmp from 127.0.0.1"
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Deleting icmp DROP from 127.0.0.1 rule"
	NFRUN_REMOVE

	tst_res TINFO "Pinging 127.0.0.1 again"
	ping -c 2 127.0.0.1 -W 1 -i 0 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$toolname blocking loopback. This is expected" \
			       "behaviour on certain distributions where" \
			       "enabling firewall drops all packets by default"
		cat tst_iptables.out
		return
	fi
	tst_res TINFO "Ping succsess"
	tst_res TPASS "$toolname can DROP packets from particular IP"
}

test3()
{
	tst_res TINFO "Use $toolname to REJECT ping request"
	tst_res TINFO "Rule to reject ping request"

	NFRUN -A INPUT -p icmp --icmp-type echo-request -d 127.0.0.1 -j \
		 REJECT > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$toolname command failed to append new rule"
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Pinging 127.0.0.1"
	ping -c 2 127.0.0.1 -W 1 -i 0 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		grep "100% packet loss" tst_iptables.out > tst_iptables.err 2>&1
		if [ $? -ne 0 ]; then
			tst_res TFAIL "$toolname did not block ping request"
			cat tst_iptables.err
			return
		else
			tst_res TINFO "Ping 127.0.0.1 not successful"
		fi
	else
		tst_res TFAIL "$toolname did not reject ping request"
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Deleting icmp request REJECT rule"
	NFRUN_REMOVE

	tst_res TINFO "Pinging 127.0.0.1 again"
	ping -c 2 127.0.0.1 -W 1 -i 0 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$toolname blocking ping requests. This is" \
			      "expected behaviour on certain distributions" \
			      "where enabling firewall drops all packets by" \
			      "default"
		cat tst_iptables.out
		return
	fi
	tst_res TINFO "Ping succsess"
	tst_res TPASS "$toolname can REJECT ping requests"
}

test4()
{
	local dport=45886
	local logprefix="${TCID}$(date +%m%d%H%M%S):"

	tst_res TINFO "Use $toolname to log packets to particular port"
	tst_res TINFO "Rule to log tcp packets to particular port"

	NFRUN -A INPUT -p tcp -d 127.0.0.1 --dport $dport -j LOG \
		 --log-prefix "$logprefix" > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$toolname command failed to append new rule"
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
				 "$toolname did not log packets to port $dport"
			cat tst_iptables.err
			return
		else
			tst_res TINFO "Packets to port $dport logged"
		fi
	else
		tst_res TFAIL "telnet to 127.0.0.1 $dport should fail"
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Deleting the rule to log"
	NFRUN_REMOVE

	tst_res TINFO "$toolname logging succsess"
	tst_res TPASS "$toolname can log packets to particular port"
}

test5()
{
	local dport=0
	local logprefix="${TCID}$(date +%m%d%H%M%S):"

	tst_res TINFO "Use $toolname to log packets to multiple ports"
	tst_res TINFO "Rule to log tcp packets to port 45801 - 45803"
	NFRUN -A INPUT -p tcp -d 127.0.0.1 --dport 45801:45803 -j LOG \
		 --log-prefix "$logprefix" > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$toolname command failed to append new rule"
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Rule to log tcp packets to port 45804 - 45806"
	NFRUN -A INPUT -p tcp -d 127.0.0.1 -m multiport --dports \
		 45804,45806,45805 -j LOG --log-prefix "$logprefix" \
		 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$toolname command failed to append new rule"
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
				tst_res TFAIL "$toolname did not log packets" \
					       "to port $dport"
				cat tst_iptables.err
				return
			else
				tst_res TINFO "Packets to port $dport logged"
			fi
		else
			tst_res TFAIL "telnet to 127.0.0.1 $dport should fail"
			cat tst_iptables.out
			return
		fi
	done

	tst_res TINFO "Flushing all rules"
	NFRUN -F > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$toolname did not flush all rules"
		cat tst_iptables.out
		return
	fi
	tst_res TINFO "$toolname logging succsess"
	tst_res TPASS "$toolname can log packets to multiple ports"
}

test6()
{
	local logcnt=0
	local logprefix="${TCID}$(date +%m%d%H%M%S):"

	tst_res TINFO "Use $toolname to log ping request with limited rate"
	tst_res TINFO "Rule to log ping request"

	NFRUN -A INPUT -p icmp --icmp-type echo-request -d 127.0.0.1 -m \
		 limit -j LOG --log-prefix "$logprefix" > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$toolname command failed to append new rule"
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "ping 127.0.0.1"
	ping -c 10 127.0.0.1 -W 1 -i 0 > tst_iptables.out 2>&1
	if [ $? -eq 0 ]; then
		sleep 2
		logcnt=$(dmesg | grep -c "$logprefix")
		if [ $logcnt -ne 5 ]; then
			tst_res TFAIL "$toolname did not log packets with" \
				      "limited rate"
			cat tst_iptables.out
			return
		else
			tst_res TINFO "ping requests logged with limited rate"
		fi
	else
		tst_res TFAIL "ping to 127.0.0.1 failed. This is expected" \
			      "behaviour on certain distributions where" \
			      "enabling firewall drops all packets by default"
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Deleting the rule to log"
	NFRUN_REMOVE

	tst_res TINFO "$toolname limited logging succsess"
	tst_res TPASS "$toolname can log packets with limited rate"
}
