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
TST_NEEDS_CMDS="grep telnet"

. tst_net.sh

NFRUN()
{
	local rule

	if [ "$use_iptables" = 1 ]; then
		ip${TST_IPV6}tables $@
	else
		$(ip${TST_IPV6}tables-translate $@ | sed 's,\\,,g')
	fi
}

NFRUN_REMOVE()
{
	if [ "$use_iptables" = 1 ]; then
		ROD ip${TST_IPV6}tables -D INPUT 1
	else
		ROD nft flush chain ip${TST_IPV6} filter INPUT
	fi
}

init()
{
	if [ "$use_iptables" = 1 ]; then
		toolname=ip${TST_IPV6}tables
		cmds="$toolname"
		tst_require_drivers ip${TST_IPV6}_tables
	else
		toolname=nft
		cmds="$toolname ip${TST_IPV6}tables-translate"
	fi

	if [ "$TST_IPV6" ];then
		loc_addr="::1"
		proto="icmpv6"
	else
		loc_addr="127.0.0.1"
		proto="icmp"
	fi

	ping_cmd="ping$TST_IPV6"
	tst_require_cmds $cmds $ping_cmd

	tst_res TINFO "INIT: Flushing all rules"
	NFRUN -F -t filter > tst_iptables.out 2>&1
	NFRUN -F -t nat > tst_iptables.out 2>&1
	NFRUN -F -t mangle > tst_iptables.out 2>&1
}

cleanup()
{
	if lsmod | grep -q "ip${TST_IPV6}_tables"; then
		NFTRUN -F -t filter > /dev/null 2>&1
		NFTRUN -F -t nat > /dev/null 2>&1
		NFTRUN -F -t mangle > /dev/null 2>&1
		rmmod -v ipt_limit ipt_multiport ipt_LOG ipt_REJECT \
			 ip${TST_IPV6}table_mangle ip${TST_IPV6}table_nat ip_conntrack \
			 ip${TST_IPV6}table_filter ip${TST_IPV6}_tables nf_nat_ipv${TST_IPVER} nf_nat \
			 nf_log_ipv${TST_IPVER} nf_log_common nf_reject_ipv${TST_IPVER} \
			 nf_conntrack_ipv${TST_IPVER} nf_defrag_ipv${TST_IPVER} nf_conntrack \
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
	local ipt_cmd="ip${TST_IPV6}tables"
	local cmd="$ipt_cmd -L -t filter"
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

	local cmd="$ipt_cmd -L -t nat"
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

	local cmd="$ipt_cmd -L -t mangle"
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

	tst_res TPASS "$ipt_cmd -L lists rules"
}

test2()
{
	tst_res TINFO "Use $toolname to DROP packets from particular IP"
	tst_res TINFO "Rule to block icmp from $loc_addr"

	NFRUN -A INPUT -s $loc_addr -p $proto -j DROP > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$toolname command failed to append new rule"
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Pinging $loc_addr"
	$ping_cmd -c 2 $loc_addr -W 1 -i 0 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		grep "100% packet loss" tst_iptables.out > tst_iptables.err 2>&1
		if [ $? -ne 0 ]; then
			tst_res TFAIL \
				 "$toolname did not block packets from loopback"
			cat tst_iptables.err
			return
		else
			tst_res TINFO "Ping $loc_addr not successful"
		fi
	else
		tst_res TFAIL "$toolname did not block $proto from $loc_addr"
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Deleting $proto DROP from $loc_addr rule"
	NFRUN_REMOVE

	tst_res TINFO "Pinging $loc_addr again"
	$ping_cmd -c 2 $loc_addr -W 1 -i 0 > tst_iptables.out 2>&1
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

	NFRUN -A INPUT -p $proto --${proto}-type echo-request -d $loc_addr -j \
		 REJECT > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$toolname command failed to append new rule"
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Pinging $loc_addr"
	$ping_cmd -c 2 $loc_addr -W 1 -i 0 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		grep "100% packet loss" tst_iptables.out > tst_iptables.err 2>&1
		if [ $? -ne 0 ]; then
			tst_res TFAIL "$toolname did not block ping request"
			cat tst_iptables.err
			return
		else
			tst_res TINFO "Ping $loc_addr not successful"
		fi
	else
		tst_res TFAIL "$toolname did not reject ping request"
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Deleting icmp request REJECT rule"
	NFRUN_REMOVE

	tst_res TINFO "Pinging $loc_addr again"
	$ping_cmd -c 2 $loc_addr -W 1 -i 0 > tst_iptables.out 2>&1
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

	NFRUN -A INPUT -p tcp -d $loc_addr --dport $dport -j LOG \
		 --log-prefix "$logprefix" > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$toolname command failed to append new rule"
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "telnet $loc_addr $dport"
	telnet $loc_addr $dport > tst_iptables.out 2>&1
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
		tst_res TFAIL "telnet to $loc_addr $dport should fail"
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
	NFRUN -A INPUT -p tcp -d $loc_addr --dport 45801:45803 -j LOG \
		 --log-prefix "$logprefix" > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$toolname command failed to append new rule"
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Rule to log tcp packets to port 45804 - 45806"
	NFRUN -A INPUT -p tcp -d $loc_addr -m multiport --dports \
		 45804,45806,45805 -j LOG --log-prefix "$logprefix" \
		 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$toolname command failed to append new rule"
		cat tst_iptables.out
		return
	fi

	for dport in 45801 45802 45803 45804 45805 45806; do
		tst_res TINFO "telnet $loc_addr $dport"
		telnet $loc_addr $dport > tst_iptables.out 2>&1
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
			tst_res TFAIL "telnet to $loc_addr $dport should fail"
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

	NFRUN -A INPUT -p $proto --$proto-type echo-request -d $loc_addr -m \
		 limit -j LOG --log-prefix "$logprefix" > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$toolname command failed to append new rule"
		cat tst_iptables.out
		return
	fi

	tst_res TINFO "Pinging $loc_addr"
	$ping_cmd -c 10 $loc_addr -W 1 -i 0 > tst_iptables.out 2>&1
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
		tst_res TFAIL "ping to $loc_addr failed. This is expected" \
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
