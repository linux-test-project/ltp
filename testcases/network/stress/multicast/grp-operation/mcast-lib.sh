#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017-2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) International Business Machines Corp., 2006
# Author: Petr Vorel <pvorel@suse.cz>
#
# Setup script for multicast stress tests.

TST_SETUP="do_setup"
TST_CLEANUP="mcast_cleanup"
TST_TESTFUNC="do_test"
TST_NEEDS_TMPDIR=1
. tst_net_stress.sh

mcast_setup4()
{
	local igmp_max_memberships="$1"

	SYSFS_IGMP_MAX_MEMBERSHIPS=$(sysctl -b net.ipv4.igmp_max_memberships)
	SYSFS_IGMP_MAX_MSF=$(sysctl -b net.ipv4.igmp_max_msf)
	SYSFS_FORCE_IGMP_VERSION=$(sysctl -b net.ipv4.conf.$(tst_iface).force_igmp_version)
	SYSFS_ALL_FORCE_IGMP_VERSION=$(sysctl -b net.ipv4.conf.all.force_igmp_version)

	[ "$igmp_max_memberships" -gt 5459 ] && tst_res TWARN \
		"\$1 shouldn't be set higher than 5459 as it's used to set /proc/sys/net/ipv4/igmp_max_memberships"

	ROD sysctl -q -w net.ipv4.igmp_max_memberships=$igmp_max_memberships
	ROD sysctl -q -w net.ipv4.igmp_max_msf=10
	ROD sysctl -q -w net.ipv4.conf.$(tst_iface).force_igmp_version=0
	ROD sysctl -q -w net.ipv4.conf.all.force_igmp_version=0
}

mcast_setup6()
{
	local default_mld_max_msf=64
	tst_kvcmp -lt '2.6.15' && default_mld_max_msf=10

	SYSCTL_ALL_FORCE_MLD_VERSION=$(sysctl -b net.ipv6.conf.all.force_mld_version)
	SYSCTL_FORCE_MLD_VERSION=$(sysctl -b net.ipv6.conf.$(tst_iface).force_mld_version)
	SYSCTL_MLD_MAX_MSF=$(sysctl -b net.ipv6.mld_max_msf)

	ROD sysctl -q -w net.ipv6.conf.all.force_mld_version=0
	ROD sysctl -q -w net.ipv6.conf.$(tst_iface).force_mld_version=0
	ROD sysctl -q -w net.ipv6.mld_max_msf=$default_mld_max_msf
}

mcast_setup()
{
	local max="$1"

	MCAST_LCMD="ns-mcast_join -f $TST_IPVER -I $(tst_iface)"

	local cmd="ns-igmp_querier"
	[ "$TST_IPV6" ] && cmd="ns-icmpv6_sender"
	MCAST_RCMD="$cmd -I $(tst_iface rhost)"

	netstress_setup

	[ "$TST_IPV6" ] && mcast_setup6 || mcast_setup4 $max
}

mcast_cleanup4()
{
	[ -n "$SYSFS_IGMP_MAX_MEMBERSHIPS" ] && sysctl -q -w net.ipv4.igmp_max_memberships=$SYSFS_IGMP_MAX_MEMBERSHIPS
	[ -n "$SYSFS_IGMP_MAX_MSF" ] && sysctl -q -w net.ipv4.igmp_max_msf=$SYSFS_IGMP_MAX_MSF
	[ -n "$SYSFS_FORCE_IGMP_VERSION" ] && sysctl -q -w net.ipv4.conf.$(tst_iface).force_igmp_version=$SYSFS_FORCE_IGMP_VERSION
	[ -n "$SYSFS_ALL_FORCE_IGMP_VERSION" ] && sysctl -q -w net.ipv4.conf.all.force_igmp_version=$SYSFS_ALL_FORCE_IGMP_VERSION
}

mcast_cleanup6()
{
	[ -n "$SYSCTL_ALL_FORCE_MLD_VERSION" ] && sysctl -q -w net.ipv6.conf.all.force_mld_version=$SYSCTL_ALL_FORCE_MLD_VERSION
	[ -n "$SYSCTL_FORCE_MLD_VERSION" ] && sysctl -q -w net.ipv6.conf.$(tst_iface).force_mld_version=$SYSCTL_FORCE_MLD_VERSION
	[ -n "$SYSCTL_MLD_MAX_MSF" ] && sysctl -q -w net.ipv6.mld_max_msf=$SYSCTL_MLD_MAX_MSF
}

mcast_cleanup()
{
	[ "$TST_IPV6" ] && mcast_cleanup6 || mcast_cleanup4

	pkill -SIGHUP -f "$MCAST_LCMD"
	tst_sleep 10ms
	pkill -9 -f "$MCAST_LCMD"

	tst_rhost_run -c "pkill -SIGHUP -f '$MCAST_RCMD'"
}

do_multicast_test_multiple_join()
{
	local num="$1"
	local mprefix="$MCAST_IPV4_ADDR_PREFIX"
	local param_multi_socket ret tmpfile

	[ "${2:-}" = true ] && param_multi_socket="-m"
	[ "$TST_IPV6" ] && mprefix="$MCAST_IPV6_ADDR_PREFIX"

	# Run a multicast join tool
	tmpfile=$$
	EXPECT_PASS $MCAST_LCMD $param_multi_socket -n $num -p $mprefix \> $tmpfile
	tst_res TINFO "joined $(grep groups $tmpfile)"

	# Send MLD / IGMP General Query from the remote host
	if [ "$TST_IPV6" ]; then
		EXPECT_RHOST_PASS $MCAST_RCMD -S $(tst_ipaddr) -m -o
	else
		EXPECT_RHOST_PASS $MCAST_RCMD -o -r 1 -m $MCAST_IPV4_ADDR
	fi
}

do_multicast_test_join_leave()
{
	local cnt define_src_addr filter params pid pids ret
	local max="$1"
	local maddr="$MCAST_IPV4_ADDR"
	[ "$TST_IPV6" ] && maddr="$MCAST_IPV6_ADDR"

	[ "$2" = true ] && define_src_addr=true

	# Send MLD / IGMP General Query from the remote host
	if [ "$TST_IPV6" ]; then
		tst_rhost_run -s -c "$MCAST_RCMD -S $(tst_ipaddr) -m -w 1000000000 -r 1000 -b"
	else
		tst_rhost_run -s -c "$MCAST_RCMD -i 1000000000 -r 1 -b"
	fi

	# Run a multicast join tool
	cnt=0
	while [ $cnt -lt $max ]; do
		if [ "$define_src_addr" ]; then
			[ $((cnt % 5)) -ne 2 ] && filter="include" || filter="exclude"
			params="-F $filter -s $(tst_ipaddr_un -c$cnt)"
		fi

		$MCAST_LCMD -l $NS_TIMES -a $maddr $params &
		pids="$! $pids"
		cnt=$((cnt + 1))
	done

	for pid in $pids; do wait $pid; done

	tst_res TPASS "test is finished successfully"
}
