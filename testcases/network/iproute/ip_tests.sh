#! /bin/sh
# Copyright (c) 2014-2016 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001
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
# Description:  Test basic functionality of ip command in route2 package
#
# Author:       Manoj Iyer, manjo@mail.utexas.edu

TST_CLEANUP=cleanup
TST_TOTAL=6
TCID="ip_tests"

. test_net.sh

rm_dummy=

init()
{
	tst_resm TINFO "inititalizing tests"
	tst_require_root
	tst_tmpdir
	tst_check_cmds cat awk ip diff

	iface=ltp_dummy
	lsmod | grep -q dummy || rm_dummy=1

	ROD ip li add $iface type dummy

	ip4_addr=${IPV4_NET16_UNUSED}.6.6
	ROD ip a add ${ip4_addr}/24 dev $iface

	cat > tst_ip02.exp <<-EOF
	1:
	link/loopback
	2:
	link/ether
	3:
	link/ether
	EOF

	if [ $? -ne 0 ]; then
		tst_brkm TBROK "can't create expected output for test02"
	fi
}

cleanup()
{
	tst_rmdir

	[ -n "$iface" -a -d /sys/class/net/$iface ] && ip li del $iface

	[ "$rm_dummy" ] && modprobe -r dummy

	# test #5
	ip route show | grep $ip4_addr && ip route del $ip4_addr
}

test01()
{
	tst_resm TINFO "test 'ip link set' command"
	tst_resm TINFO "changing mtu size of $iface device"

	MTUSZ_BAK=$(cat /sys/class/net/${iface}/mtu)
	ip link set ${iface} mtu 1281
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "ip command failed"
		return
	fi

	MTUSZ=$(cat /sys/class/net/${iface}/mtu)
	if [ $MTUSZ -eq 1281 ]; then
		tst_resm TPASS "successfully changed mtu size"
		ip link set $iface mtu $MTUSZ_BAK
	else
		tst_resm TFAIL "MTU value set to $MTUSZ, but expected 1281"
	fi
}

test02()
{
	tst_resm TINFO "test 'ip link show' command (list device attributes)"

	ip link show $iface | grep $iface > /dev/null
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "'ip link show $iface' command failed"
		return
	fi

	tst_resm TPASS "$iface correctly listed"
}

test03()
{
	tst_resm TINFO "test 'ip addr' command with loopback dev"
	tst_resm TINFO "add a new protocol address to the device"

	ip addr add 127.6.6.6/24 dev lo
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "'ip addr add' command failed"
		return
	fi

	tst_resm TINFO "show protocol address"
	ip addr show dev lo | grep 127.6.6.6 > /dev/null
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "'ip addr show' command failed"
		return
	fi

	tst_resm TINFO "delete protocol address"
	ip addr del 127.6.6.6/24 dev lo
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "'ip addr del' command failed"
		return
	fi

	ip addr show dev lo | grep 127.6.6.6 > /dev/null
	if [ $? -eq 0 ]; then
		tst_resm TFAIL "ip addr del command failed"
		return
	fi

	tst_resm TPASS "'ip addr' command successfully tested"
}

test04()
{
	tst_resm TINFO "test 'ip neigh' command"
	tst_resm TINFO "add a new neighbor (or replace existed)"
	ip neigh replace 127.6.6.6 lladdr 00:00:00:00:00:00 dev lo nud reachable
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "'ip neigh replace' command failed"
		return
	fi

	tst_resm TINFO "show all neighbor entries in arp tables"
	cat > tst_ip.exp <<-EOF
127.6.6.6 dev lo lladdr 00:00:00:00:00:00 REACHABLE
	EOF

	ip neigh show 127.6.6.6 | head -n1 > tst_ip.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "'ip neigh show' command failed"
		return
	fi

	diff -iwB tst_ip.out tst_ip.exp
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "expected output differs from actual output"
		return
	fi

	tst_resm TINFO "delete neighbor from the arp table"

	ip neigh del 127.6.6.6 dev lo
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "'ip neigh del' command failed"
		return
	fi

	ip neigh show | grep 127.6.6.6 | grep -v ' FAILED$' > /dev/null
	if [ $? -eq 0 ]; then
		tst_resm TFAIL "127.6.6.6 still listed in arp"
		return
	fi

	tst_resm TPASS "'ip neigh' command successfully tested"
}

test05()
{
	tst_resm TINFO "test 'ip route add/del' commands"

	ROD ip route add $ip4_addr via 127.0.0.1

	tst_resm TINFO "show all route entries in route table"

	# create expected output file.
	cat > tst_ip.exp <<-EOF
$ip4_addr via 127.0.0.1 dev lo
	EOF

	ip route show | grep "$ip4_addr via 127.0.0.1 dev lo" > tst_ip.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "'ip route show' command failed"
		return
	fi

	diff -iwB tst_ip.out tst_ip.exp
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "'ip route show' did not list new route"
		return
	fi

	tst_resm TINFO "delete route from the route table"

	ROD ip route del $ip4_addr via 127.0.0.1

	ip route show | grep 127.0.0.1 > /dev/null
	if [ $? -eq 0 ]; then
		tst_resm TFAIL "route not deleted"
		return
	fi

	tst_resm TPASS "'ip route' command successfully tested"
}

test06()
{
	tst_resm TINFO "test 'ip maddr add/del' commands"
	tst_resm TINFO "adding a new multicast addr"

	ip maddr add 66:66:00:00:00:66 dev $iface
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "ip maddr add command failed"
		return
	fi

	tst_resm TINFO "show all multicast addr entries"

	cat > tst_ip.exp <<-EOF
        link  66:66:00:00:00:66 static
	EOF

	ip maddr show | grep "66:66:00:00:00:66" > tst_ip.out 2>&1
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "'ip maddr show' command failed"
		return
	fi

	diff -iwB tst_ip.out tst_ip.exp
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "multicast addr not added to $iface"
		return
	fi

	tst_resm TINFO "delete multicast address"

	ip maddr del 66:66:00:00:00:66 dev $iface
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "'ip maddr del' command failed"
		return
	fi

	ip maddr show | grep "66:66:00:00:00:66" > /dev/null
	if [ $? -eq 0 ]; then
		tst_resm TFAIL "66:66:00:00:00:66 is not deleted"
		return
	fi

	tst_resm TPASS "'ip maddr' command successfully tested"
}

init

test01
test02
test03
test04
test05
test06

tst_exit
