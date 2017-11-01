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
## along with this program;  if not, write to the Free Software Foundation,   ##
## Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           ##
##                                                                            ##
################################################################################
#  Author:	Jan 20 2004 Hubert Lin <linux02NOSPAAAM@tw.ibm.com>
#				       <hubertNOSPAAAM@symbio.com.tw>

export TCID="iptables"
export TST_TOTAL=6

. test.sh

init()
{
	tst_tmpdir

	tst_resm TINFO "INIT: Inititalizing tests."

	modprobe ip_tables
	if [ $? -ne 0 ]; then
		iptables -L > tst_iptables.out 2>&1
		if [ $? -ne 0 ]; then
			tst_brkm TBROK "no iptables support in kernel."
		fi
	fi

	tst_resm TINFO "INIT: Flushing all rules."
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
	tst_rmdir
}

test01()
{
	local chaincnt=0

	local cmd="iptables -L -t filter"
	tst_resm TINFO "$cmd will list all rules in table filter."
	$cmd > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "$cmd failed to list rules."
		cat tst_iptables.out
		return
	else
		chaincnt=$(grep -c Chain tst_iptables.out)
		if [ $chaincnt -lt 3 ]; then
			tst_resm TFAIL "$cmd failed to list rules."
			cat tst_iptables.out
			return
		else
			tst_resm TINFO "$cmd lists rules."
		fi
	fi

	local cmd="iptables -L -t nat"
	tst_resm TINFO "$cmd will list all rules in table nat."
	$cmd > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "$cmd failed to list rules."
		cat tst_iptables.out
		return
	else
		chaincnt=$(grep -c Chain tst_iptables.out)
		if [ $chaincnt -lt 3 ]; then
			tst_resm TFAIL "$cmd failed to list rules."
			cat tst_iptables.out
			return
		else
			tst_resm TINFO "$cmd lists rules."
		fi
	fi

	local cmd="iptables -L -t mangle"
	tst_resm TINFO "$cmd will list all rules in table mangle."
	$cmd > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "$cmd failed to list rules."
		cat tst_iptables.out
		return
	else
		chaincnt=$(grep -c Chain tst_iptables.out)
		if [ $chaincnt -lt 5 ]; then
			tst_resm TFAIL "$cmd failed to list rules."
			cat tst_iptables.out
		else
			tst_resm TINFO "$cmd lists rules."
		fi
	fi

	tst_resm TPASS "iptables -L lists rules."
}

test02()
{
	tst_resm TINFO "Use iptables to DROP packets from particular IP"
	tst_resm TINFO "Rule to block icmp from 127.0.0.1"

	iptables -A INPUT -s 127.0.0.1 -p icmp -j DROP > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "iptables command failed to append new rule."
		cat tst_iptables.out
		return
	fi

	tst_resm TINFO "Pinging 127.0.0.1"
	ping -c 2 127.0.0.1 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		grep "100% packet loss" tst_iptables.out > tst_iptables.err 2>&1
		if [ $? -ne 0 ]; then
			tst_resm TFAIL \
				 "iptables did not block packets from loopback"
			cat tst_iptables.err
			return
		else
			tst_resm TINFO "Ping 127.0.0.1 not successful."
		fi
	else
		tst_resm TFAIL "iptables did not block icmp from 127.0.0.1"
		cat tst_iptables.out
		return
	fi

	tst_resm TINFO "Deleting icmp DROP from 127.0.0.1 rule."
	iptables -D INPUT 1 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "iptables did not remove the rule."
		cat tst_iptables.out
		return
	fi
	tst_resm TINFO "Pinging 127.0.0.1 again"
	ping -c 2 127.0.0.1 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "iptables blocking loopback. This is expected" \
			       "behaviour on certain distributions where" \
			       "enabling firewall drops all packets by default."
		cat tst_iptables.out
		return
	fi
	tst_resm TINFO "Ping succsess"
	tst_resm TPASS "iptables can DROP packets from particular IP."
}

test03()
{
	tst_resm TINFO "Use iptables to REJECT ping request."
	tst_resm TINFO "Rule to reject ping request."

	iptables -A INPUT -p icmp --icmp-type echo-request -d 127.0.0.1 -j \
		 REJECT > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "iptables command failed to append new rule."
		cat tst_iptables.out
		return
	fi

	tst_resm TINFO "Pinging 127.0.0.1"
	ping -c 2 127.0.0.1 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		grep "100% packet loss" tst_iptables.out > tst_iptables.err 2>&1
		if [ $? -ne 0 ]; then
			tst_resm TFAIL "iptables did not block ping request."
			cat tst_iptables.err
			return
		else
			tst_resm TINFO "Ping 127.0.0.1 not successful."
		fi
	else
		tst_resm TFAIL "iptables did not reject ping request."
		cat tst_iptables.out
		return
	fi

	tst_resm TINFO "Deleting icmp request REJECT rule."
	iptables -D INPUT 1 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "iptables did not remove the rule."
		cat tst_iptables.out
		return
	fi
	tst_resm TINFO "Pinging 127.0.0.1 again"
	ping -c 2 127.0.0.1 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "iptables blocking ping requests. This is" \
			       "expected behaviour on certain distributions" \
			       "where enabling firewall drops all packets by" \
			       "default."
		cat tst_iptables.out
		return
	fi
	tst_resm TINFO "Ping succsess"
	tst_resm TPASS "iptables can REJECT ping requests."
}

test04()
{
	local dport=45886
	local logprefix="$TCID-$(date +%m%d%H%M%S):"

	tst_resm TINFO "Use iptables to log packets to particular port."
	tst_resm TINFO "Rule to log tcp packets to particular port."

	iptables -A INPUT -p tcp -d 127.0.0.1 --dport $dport -j LOG \
		 --log-prefix "$logprefix" > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "iptables command failed to append new rule."
		cat tst_iptables.out
		return
	fi

	tst_resm TINFO "telnet 127.0.0.1 $dport"
	telnet 127.0.0.1 $dport > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		sleep 2
		dmesg | grep "$logprefix" > tst_iptables.err 2>&1
		if [ $? -ne 0 ]; then
			tst_resm TFAIL \
				 "iptables did not log packets to port $dport"
			cat tst_iptables.err
			return
		else
			tst_resm TINFO "Packets to port $dport logged."
		fi
	else
		tst_resm TFAIL "telnet to 127.0.0.1 $dport should fail."
		cat tst_iptables.out
		return
	fi

	tst_resm TINFO "Deleting the rule to log."
	iptables -D INPUT 1 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "iptables did not remove the rule."
		cat tst_iptables.out
		return
	fi
	tst_resm TINFO "iptables logging succsess"
	tst_resm TPASS "iptables can log packets to particular port."
}

test05()
{
	local dport=0
	local logprefix="$TCID-$(date +%m%d%H%M%S):"

	tst_resm TINFO "Use iptables to log packets to multiple ports."
	tst_resm TINFO "Rule to log tcp packets to port 45801 - 45803."
	iptables -A INPUT -p tcp -d 127.0.0.1 --dport 45801:45803 -j LOG \
		 --log-prefix "$logprefix" > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "iptables command failed to append new rule."
		cat tst_iptables.out
		return
	fi

	tst_resm TINFO "Rule to log tcp packets to port 45804 - 45806."
	iptables -A INPUT -p tcp -d 127.0.0.1 -m multiport --dports \
		 45804,45806,45805 -j LOG --log-prefix "$logprefix" \
		 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "iptables command failed to append new rule."
		cat tst_iptables.out
		return
	fi

	for dport in 45801 45802 45803 45804 45805 45806; do
		tst_resm TINFO "telnet 127.0.0.1 $dport"
		telnet 127.0.0.1 $dport > tst_iptables.out 2>&1
		if [ $? -ne 0 ]; then
			sleep 2
			dmesg | grep "$logprefix" | grep "=$dport " \
				> tst_iptables.err 2>&1
			if [ $? -ne 0 ]; then
				tst_resm TFAIL "iptables did not log packets" \
					       "to port $dport"
				cat tst_iptables.err
				return
			else
				tst_resm TINFO "Packets to port $dport logged."
			fi
		else
			tst_res TFAIL "telnet to 127.0.0.1 $dport should fail."
			cat tst_iptables.out
			return
		fi
	done

	tst_resm TINFO "Flushing all rules."
	iptables -F > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "iptables did not flush all rules."
		cat tst_iptables.out
		return
	fi
	tst_resm TINFO "iptables logging succsess"
	tst_resm TPASS "iptables can log packets to multiple ports."
}

test06()
{
	local logcnt=0
	local logprefix="$TCID-$(date +%m%d%H%M%S):"

	tst_resm TINFO "Use iptables to log ping request with limited rate."
	tst_resm TINFO "Rule to log ping request."

	iptables -A INPUT -p icmp --icmp-type echo-request -d 127.0.0.1 -m \
		 limit -j LOG --log-prefix "$logprefix" > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "iptables command failed to append new rule."
		cat tst_iptables.out
		return
	fi

	tst_resm TINFO "ping 127.0.0.1"
	ping -c 10 127.0.0.1 > tst_iptables.out 2>&1
	if [ $? -eq 0 ]; then
		sleep 2
		logcnt=$(dmesg | grep -c "$logprefix")
		if [ $logcnt -ne 5 ]; then
			tst_resm TFAIL "iptables did not log packets with" \
				       "limited rate."
			cat tst_iptables.out
			return
		else
			tst_resm TINFO "ping requests logged with limited rate."
		fi
	else
		tst_resm TFAIL "ping to 127.0.0.1 failed. This is expected" \
			       "behaviour on certain distributions where" \
			       "enabling firewall drops all packets by default."
		cat tst_iptables.out
		return
	fi

	tst_resm TINFO "Deleting the rule to log."
	iptables -D INPUT 1 > tst_iptables.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "iptables did not remove the rule."
		cat tst_iptables.out
		return
	fi
	tst_resm TINFO "iptables limited logging succsess"
	tst_resm TPASS "iptables can log packets with limited rate."
}

init
TST_CLEANUP=cleanup

test01
test02
test03
test04
test05
test06

tst_exit
