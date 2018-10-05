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
	v) VERSION=$2;;
	t) SOCKET_TYPE=$2;;
	esac
}

TST_OPTS="v:t:"
TST_PARSE_ARGS=nfs_parse_args
TST_USAGE=nfs_usage
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="${TST_NEEDS_CMDS:-mount exportfs}"
TST_SETUP="${TST_SETUP:-nfs_setup}"
TST_CLEANUP="${TST_CLEANUP:-nfs_cleanup}"

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

		local_dir="$TST_TMPDIR/$i/$n"
		remote_dir="$TST_TMPDIR/$i/$type"

		mkdir -p $local_dir

		tst_rhost_run -c "test -d $remote_dir"
		if [ "$?" -ne 0  ]; then
			tst_rhost_run -s -c "mkdir -p $remote_dir"
			tst_rhost_run -s -c "exportfs -i -o no_root_squash,rw \
				*:$remote_dir"
		fi

		opts="-o proto=$type,vers=$i"

		if [ $TST_IPV6 ]; then
			mount_dir="[$(tst_ipaddr rhost)]:$remote_dir"
		else
			mount_dir="$(tst_ipaddr rhost):$remote_dir"
		fi


		tst_res TINFO "Mounting NFS '$mount_dir'"
		tst_res TINFO "to '$local_dir' with options '$opts'"

		ROD mount -t nfs $opts $mount_dir $local_dir

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
