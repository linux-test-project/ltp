#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015-2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001

VERSION=${VERSION:=3}
NFILES=${NFILES:=1000}
SOCKET_TYPE="${SOCKET_TYPE:-udp}"
NFS_TYPE=${NFS_TYPE:=nfs}

nfs_usage()
{
	echo "-t x    Socket type, tcp or udp, default is udp"
	echo "-v x    NFS version, default is '3'"
}

nfs_parse_args()
{
	case "$1" in
	v) VERSION="$(echo $2 | tr ',' ' ')";;
	t) SOCKET_TYPE="$(echo $2 | tr ',' ' ')";;
	esac
}

TST_OPTS="v:t:"
TST_PARSE_ARGS=nfs_parse_args
TST_USAGE=nfs_usage
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="$TST_NEEDS_CMDS mount exportfs"
TST_SETUP="${TST_SETUP:-nfs_setup}"
TST_CLEANUP="${TST_CLEANUP:-nfs_cleanup}"

# When set and test is using netns ($TST_USE_NETNS set) NFS traffic will go
# through lo interface instead of ltp_ns_veth* netns interfaces (useful for
# debugging whether test failures are related to veth/netns).
LTP_NFS_NETNS_USE_LO=${LTP_NFS_NETNS_USE_LO:-}

. tst_net.sh

get_socket_type()
{
	local t
	local k=0
	for t in $SOCKET_TYPE; do
		if [ "$k" -eq "$1" ]; then
			echo "${t}${TST_IPV6}"
			return
		fi
		k=$(( k + 1 ))
	done
}

nfs_server_udp_enabled()
{
	local config f

	tst_rhost_run -c "[ -f /etc/nfs.conf ]" || return 0
	config=$(tst_rhost_run -c 'for f in $(grep ^include.*= '/etc/nfs.conf' | cut -d = -f2); do [ -f $f ] && printf "$f "; done')

	tst_rhost_run -c "grep -q '^[# ]*udp *= *y' /etc/nfs.conf $config"
}

nfs_setup_server()
{
	local export_cmd="exportfs -i -o fsid=$$,no_root_squash,rw *:$remote_dir"

	if ! tst_rhost_run -c "test -d $remote_dir"; then
		tst_rhost_run -s -c "mkdir -p $remote_dir; $export_cmd"
	fi
}

nfs_mount()
{
	local host_type=rhost
	local mount_dir

	[ -n "$LTP_NETNS" ] && host_type=

	if [ $TST_IPV6 ]; then
		mount_dir="[$(tst_ipaddr $host_type)]:$remote_dir"
	else
		mount_dir="$(tst_ipaddr $host_type):$remote_dir"
	fi

	local mnt_cmd="mount -t nfs $opts $mount_dir $local_dir"

	tst_res TINFO "Mounting NFS: $mnt_cmd"
	if [ -n "$LTP_NETNS" ] && [ -z "$LTP_NFS_NETNS_USE_LO" ]; then
		tst_rhost_run -s -c "$mnt_cmd"
		return
	fi

	ROD $mnt_cmd
}

nfs_setup()
{
	# Check if current filesystem is NFS
	if [ "$(stat -f . | grep "Type: nfs")" ]; then
		tst_brk TCONF "Cannot run nfs-stress test on mounted NFS"
	fi

	local i
	local type
	local n=0
	local opts
	local local_dir
	local remote_dir
	local mount_dir

	for i in $VERSION; do
		type=$(get_socket_type $n)
		tst_res TINFO "setup NFSv$i, socket type $type"

		if [ "$type" = "udp" -o "$type" = "udp6" ] && ! nfs_server_udp_enabled; then
			tst_brk TCONF "UDP support disabled on NFS server"
		fi

		local_dir="$TST_TMPDIR/$i/$n"
		remote_dir="$TST_TMPDIR/$i/$type"
		mkdir -p $local_dir

		nfs_setup_server

		opts="-o proto=$type,vers=$i"
		nfs_mount

		n=$(( n + 1 ))
	done

	if [ "$n" -eq 1 ]; then
		cd ${VERSION}/0
	fi
}

nfs_cleanup()
{
	tst_res TINFO "Cleaning up testcase"
	cd $LTPROOT

	local i
	local type
	local local_dir
	local remote_dir

	local n=0
	for i in $VERSION; do
		local_dir="$TST_TMPDIR/$i/$n"
		grep -q "$local_dir" /proc/mounts && umount $local_dir
		n=$(( n + 1 ))
	done

	n=0
	for i in $VERSION; do
		type=$(get_socket_type $n)
		remote_dir="$TST_TMPDIR/$i/$type"
		tst_rhost_run -c "test -d $remote_dir && exportfs -u *:$remote_dir"
		tst_rhost_run -c "test -d $remote_dir && rm -rf $remote_dir"
		n=$(( n + 1 ))
	done
}
