# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
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

nfs_setup()
{
	VERSION=${VERSION:=3}
	NFILES=${NFILES:=1000}
	SOCKET_TYPE="${SOCKET_TYPE:=udp}${TST_IPV6}"
	NFS_TYPE=${NFS_TYPE:=nfs}

	tst_check_cmds mount exportfs

	tst_tmpdir

	# Check if current filesystem is NFS
	if [ "$(stat -f . | grep "Type: nfs")" ]; then
		tst_brkm TCONF "Cannot run nfs-stress test on mounted NFS"
	fi

	tst_resm TINFO "NFS_TYPE: $NFS_TYPE, NFS VERSION: $VERSION"
	tst_resm TINFO "NFILES: $NFILES, SOCKET_TYPE: $SOCKET_TYPE"

	if [ "$NFS_TYPE" != "nfs4" ]; then
		OPTS=${OPTS:="-o proto=$SOCKET_TYPE,vers=$VERSION "}
	fi

	tst_rhost_run -s -c "mkdir -p $TST_TMPDIR"

	if [ $TST_IPV6 ]; then
		REMOTE_DIR="[$(tst_ipaddr rhost)]:$TST_TMPDIR"
	else
		REMOTE_DIR="$(tst_ipaddr rhost):$TST_TMPDIR"
	fi

	if [ "$NFS_TYPE" = "nfs4" ]; then
		tst_rhost_run -s -c "mkdir -p /export$TST_TMPDIR"
		tst_rhost_run -s -c "mount --bind $TST_TMPDIR /export$TST_TMPDIR"
		tst_rhost_run -s -c "exportfs -o no_root_squash,rw,nohide,\
			insecure,no_subtree_check *:$TST_TMPDIR"
	else
		tst_rhost_run -s -c "exportfs -i -o no_root_squash,rw \
			*:$TST_TMPDIR"
	fi

	tst_resm TINFO "Mounting NFS '$REMOTE_DIR' with options '$OPTS'"
	ROD mount -t $NFS_TYPE $OPTS $REMOTE_DIR $TST_TMPDIR
	cd $TST_TMPDIR
}

nfs_cleanup()
{
	tst_resm TINFO "Cleaning up testcase"
	cd $LTPROOT
	grep -q "$TST_TMPDIR" /proc/mounts && umount $TST_TMPDIR

	tst_rhost_run -c "exportfs -u *:$TST_TMPDIR"
	tst_rhost_run -c "rm -rf $TST_TMPDIR"
}
