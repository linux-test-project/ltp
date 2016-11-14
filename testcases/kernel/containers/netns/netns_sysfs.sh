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
DUMMYDEV_HOST="dummy_test0"
DUMMYDEV="dummy_test1"
. test.sh

if tst_kvcmp -lt "2.6.35"; then
	tst_brkm TCONF "sysfs is not mount namespace aware for kernels older than 2.6.35"
fi

setns_check
if [ $? -eq 32 ]; then
	tst_brkm TCONF "setns not supported"
fi

cleanup()
{
	tst_rmdir
	ip link del $DUMMYDEV_HOST 2>/dev/null
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

ip link add $DUMMYDEV_HOST type dummy || \
	tst_brkm TBROK "failed to add a new (host) dummy device"

ns_exec $NS_HANDLE $NS_TYPE mount --make-rprivate /sys
ns_exec $NS_HANDLE $NS_TYPE ip link add $DUMMYDEV type dummy || \
	tst_brkm TBROK "failed to add a new dummy device"
ns_exec $NS_HANDLE $NS_TYPE mount -t sysfs none /sys 2>/dev/null


# TEST CASE #1
ns_exec $NS_HANDLE $NS_TYPE test -e /sys/class/net/$DUMMYDEV
if [ $? -eq 0 ]; then
	tst_resm TPASS "sysfs in new namespace has $DUMMYDEV interface"
else
	tst_resm TFAIL "sysfs in new namespace does not have $DUMMYDEV interface"
fi


# TEST CASE #2
ns_exec $NS_HANDLE $NS_TYPE test -e /sys/class/net/$DUMMYDEV_HOST
if [ $? -ne 0 ]; then
	tst_resm TPASS "sysfs in new namespace does not have $DUMMYDEV_HOST interface"
else
	tst_resm TFAIL "sysfs in new namespace contains $DUMMYDEV_HOST interface"
fi


# TEST CASE #3
test -e /sys/class/net/$DUMMYDEV
if [ $? -ne 0 ]; then
	tst_resm TPASS "sysfs not affected by a separate namespace"
else
	tst_resm TFAIL "sysfs affected by a separate namespace"
fi


tst_exit
