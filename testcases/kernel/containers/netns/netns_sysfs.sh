#!/bin/sh
#==============================================================================
# Copyright (c) 2015 Red Hat, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of version 2 the GNU General Public License as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Written by Matus Marhefka <mmarhefk@redhat.com>
#
#==============================================================================
#
# Tests that a separate network namespace cannot affect sysfs contents
# of the main namespace.
#==============================================================================

TCID="netns_sysfs"
TST_TOTAL=3
NS_TYPE="net,mnt"
DUMMYDEV="dummy_test0"
. test.sh

setns_check
if [ $? -eq 32 ]; then
	tst_brkm TCONF "setns not supported"
fi

cleanup()
{
	tst_rmdir
	ip link del $DUMMYDEV 2>/dev/null
	kill -9 $NS_HANDLE 2>/dev/null
}

tst_tmpdir
NS_HANDLE=$(ns_create $NS_TYPE)
if [ $? -eq 1 ]; then
	tst_resm TINFO "$NS_HANDLE"
	tst_brkm TBROK "unable to create a new network namespace"
fi
TST_CLEANUP=cleanup

# exclude dummy0 (dummy1, etc.) from comparison as it gets automatically created
# by the dummy device driver upon insmod/modprobe (during ip link add)
ls /sys/class/net | grep -v 'dummy[0-9]\+' >sysfs_before

ns_exec $NS_HANDLE $NS_TYPE mount --make-rprivate /sys
ns_exec $NS_HANDLE $NS_TYPE ip link add $DUMMYDEV type dummy || \
	tst_brkm TBROK "failed to add a new dummy device"
ns_exec $NS_HANDLE $NS_TYPE mount -t sysfs none /sys 2>/dev/null


# TEST CASE #1
ns_exec $NS_HANDLE $NS_TYPE test -d /sys/class/net/$DUMMYDEV
if [ $? -eq 0 ]; then
	tst_resm TPASS "sysfs in new namespace has $DUMMYDEV interface"
else
	tst_resm TFAIL "sysfs in new namespace does not have $DUMMYDEV interface"
fi


# TEST CASE #2
res=0
for d in $(ns_exec $NS_HANDLE $NS_TYPE ls /sys/class/net/); do
	case "$d" in
		lo|$DUMMYDEV)
			;;
		*)
			tst_resm TINFO "sysfs in new namespace should not contain: $d"
			res=1
			;;
	esac
done
if [ $res -eq 0 ]; then
	tst_resm TPASS "sysfs in new namespace has only lo and $DUMMYDEV interfaces"
else
	tst_resm TFAIL "sysfs in new namespace has more than lo and $DUMMYDEV interfaces"
fi


# TEST CASE #3
ls /sys/class/net | grep -v 'dummy[0-9]\+' >sysfs_after
diff sysfs_before sysfs_after
if [ $? -eq 0 ]; then
	tst_resm TPASS "sysfs not affected by a separate namespace"
else
	tst_resm TFAIL "sysfs affected by a separate namespace"
fi


tst_exit
