#! /bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2014-2019 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines Corp., 2001
# Author:       Manoj Iyer, manjo@mail.utexas.edu
#
# Description:  Test basic functionality of ip command in route2 package

TST_CNT=6
TST_SETUP="init"
TST_TESTFUNC="test"
TST_CLEANUP="cleanup"
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="awk cat diff ip lsmod modprobe"
TST_NEEDS_DRIVERS="dummy"


rm_dummy=

init()
{
	tst_res TINFO "inititalizing tests"

	iface=ltp_dummy
	lsmod | grep -q dummy || rm_dummy=1

	ROD ip link add $iface type dummy

	ip4_addr=${IPV4_NET16_UNUSED}.6.6
	ROD ip addr add ${ip4_addr}/24 dev $iface

	cat > tst_ip02.exp <<-EOF
	1:
	link/loopback
	2:
	link/ether
	3:
	link/ether
	EOF

	if [ $? -ne 0 ]; then
		tst_brk TBROK "can't create expected output for test02"
	fi
}

cleanup()
{
	[ -n "$iface" -a -d /sys/class/net/$iface ] && ip link del $iface

	[ "$rm_dummy" ] && modprobe -r dummy

	# test #5
	[ "$ip4_addr" ] && ip route show | grep -q $ip4_addr && ip route del $ip4_addr
}

test1()
{
	tst_res TINFO "test 'ip link set' command"
	tst_res TINFO "changing mtu size of $iface device"

	MTUSZ_BAK=$(cat /sys/class/net/${iface}/mtu)
	ip link set ${iface} mtu 1281
	if [ $? -ne 0 ]; then
		tst_res TFAIL "ip command failed"
		return
	fi

	MTUSZ=$(cat /sys/class/net/${iface}/mtu)
	if [ $MTUSZ -eq 1281 ]; then
		tst_res TPASS "successfully changed mtu size"
		ip link set $iface mtu $MTUSZ_BAK
	else
		tst_res TFAIL "MTU value set to $MTUSZ, but expected 1281"
	fi
}

test2()
{
	tst_res TINFO "test 'ip link show' command (list device attributes)"

	ip link show $iface | grep $iface > /dev/null
	if [ $? -ne 0 ]; then
		tst_res TFAIL "'ip link show $iface' command failed"
		return
	fi

	tst_res TPASS "$iface correctly listed"
}

test3()
{
	tst_res TINFO "test 'ip addr' command with loopback dev"
	tst_res TINFO "add a new protocol address to the device"

	ip addr add 127.6.6.6/24 dev lo
	if [ $? -ne 0 ]; then
		tst_res TFAIL "'ip addr add' command failed"
		return
	fi

	tst_res TINFO "show protocol address"
	ip addr show dev lo | grep 127.6.6.6 > /dev/null
	if [ $? -ne 0 ]; then
		tst_res TFAIL "'ip addr show' command failed"
		return
	fi

	tst_res TINFO "delete protocol address"
	ip addr del 127.6.6.6/24 dev lo
	if [ $? -ne 0 ]; then
		tst_res TFAIL "'ip addr del' command failed"
		return
	fi

	ip addr show dev lo | grep 127.6.6.6 > /dev/null
	if [ $? -eq 0 ]; then
		tst_res TFAIL "ip addr del command failed"
		return
	fi

	tst_res TPASS "'ip addr' command successfully tested"
}

test4()
{
	local taddr="$(tst_ipaddr_un)"
	local tdev="$(tst_iface)"
	tst_res TINFO "test 'ip neigh' command"
	tst_res TINFO "add a new neighbor (or replace existed)"
	ip neigh replace $taddr lladdr 00:00:00:00:00:00 dev $tdev nud reachable
	if [ $? -ne 0 ]; then
		tst_res TFAIL "'ip neigh replace' command failed"
		return
	fi

	tst_res TINFO "show all neighbor entries in arp tables"
	echo "$taddr dev $tdev lladdr 00:00:00:00:00:00 REACHABLE" > tst_ip.exp

	ip neigh show $taddr | head -n1 > tst_ip.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "'ip neigh show' command failed"
		return
	fi

	diff -iwB tst_ip.out tst_ip.exp
	if [ $? -ne 0 ]; then
		tst_res TFAIL "expected output differs from actual output"
		return
	fi

	tst_res TINFO "delete neighbor from the arp table"

	ip neigh del $taddr dev $tdev
	if [ $? -ne 0 ]; then
		tst_res TFAIL "'ip neigh del' command failed"
		return
	fi

	ip neigh show | grep $taddr | grep -v ' FAILED$' > /dev/null
	if [ $? -eq 0 ]; then
		tst_res TFAIL "$taddr still listed in arp"
		return
	fi

	tst_res TPASS "'ip neigh' command successfully tested"
}

test5()
{
	tst_res TINFO "test 'ip route add/del' commands"

	ROD ip route add $ip4_addr via 127.0.0.1

	tst_res TINFO "show all route entries in route table"

	# create expected output file.
	cat > tst_ip.exp <<-EOF
$ip4_addr via 127.0.0.1 dev lo
	EOF

	ip route show | grep "$ip4_addr via 127\.0\.0\.1 dev lo" > tst_ip\.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "'ip route show' command failed"
		return
	fi

	diff -iwB tst_ip.out tst_ip.exp
	if [ $? -ne 0 ]; then
		tst_res TFAIL "'ip route show' did not list new route"
		return
	fi

	tst_res TINFO "delete route from the route table"

	ROD ip route del $ip4_addr via 127.0.0.1

	ip route show | grep -q "$ip4_addr via 127\.0\.0\.1 dev lo"
	if [ $? -eq 0 ]; then
		tst_res TFAIL "route not deleted"
		return
	fi

	tst_res TPASS "'ip route' command successfully tested"
}

test6()
{
	tst_res TINFO "test 'ip maddr add/del' commands"
	tst_res TINFO "adding a new multicast addr"

	ip maddr add 66:66:00:00:00:66 dev $iface
	if [ $? -ne 0 ]; then
		tst_res TFAIL "ip maddr add command failed"
		return
	fi

	tst_res TINFO "show all multicast addr entries"

	cat > tst_ip.exp <<-EOF
        link  66:66:00:00:00:66 static
	EOF

	ip maddr show | grep "66:66:00:00:00:66" > tst_ip.out 2>&1
	if [ $? -ne 0 ]; then
		tst_res TFAIL "'ip maddr show' command failed"
		return
	fi

	diff -iwB tst_ip.out tst_ip.exp
	if [ $? -ne 0 ]; then
		tst_res TFAIL "multicast addr not added to $iface"
		return
	fi

	tst_res TINFO "delete multicast address"

	ip maddr del 66:66:00:00:00:66 dev $iface
	if [ $? -ne 0 ]; then
		tst_res TFAIL "'ip maddr del' command failed"
		return
	fi

	ip maddr show | grep "66:66:00:00:00:66" > /dev/null
	if [ $? -eq 0 ]; then
		tst_res TFAIL "66:66:00:00:00:66 is not deleted"
		return
	fi

	tst_res TPASS "'ip maddr' command successfully tested"
}

. tst_net.sh
tst_run
