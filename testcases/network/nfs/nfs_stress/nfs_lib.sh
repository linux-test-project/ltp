#!/bin/sh
# Copyright (c) 2015-2016 Oracle and/or its affiliates. All Rights Reserved.
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
# along with this program. If not, see <http://www.gnu.org/licenses/>.

VERSION=${VERSION:=3}
NFILES=${NFILES:=1000}
SOCKET_TYPE="${SOCKET_TYPE:-udp}"
NFS_TYPE=${NFS_TYPE:=nfs}

while getopts :ht:v:6 opt; do
	case "$opt" in
	h)
		echo "Usage:"
		echo "h        help"
		echo "t x      socket type, tcp or udp, default is udp"
		echo "v x      NFS version, default is '3'"
		echo "6        run over IPv6"
		exit 0
	;;
	v) VERSION=$OPTARG ;;
	t) SOCKET_TYPE=$OPTARG ;;
	6) # skip, test_net library already processed it
	;;
	*)
		tst_brkm TBROK "unknown option: $opt"
	;;
	esac
done

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
	tst_check_cmds mount exportfs

	tst_tmpdir

	# Check if current filesystem is NFS
	if [ "$(stat -f . | grep "Type: nfs")" ]; then
		tst_brkm TCONF "Cannot run nfs-stress test on mounted NFS"
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
		tst_resm TINFO "setup NFSv$i, socket type $type"

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


		tst_resm TINFO "Mounting NFS '$mount_dir'"
		tst_resm TINFO "to '$local_dir' with options '$opts'"

		ROD mount -t nfs $opts $mount_dir $local_dir

		n=$(( n + 1 ))
	done

	if [ "$n" -eq 1 ]; then
		cd ${VERSION}/0
	fi
}

nfs_cleanup()
{
	tst_resm TINFO "Cleaning up testcase"
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
