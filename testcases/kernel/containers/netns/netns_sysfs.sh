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
TST_TOTAL=2
. test.sh

cleanup()
{
	tst_rmdir
	ip link del dummy0 2>/dev/null
	kill -9 $NS_HANDLE 2>/dev/null
}

tst_tmpdir
NS_HANDLE=$(ns_create net mnt)
if [ $? -eq 1 ]; then
	tst_resm TINFO "$NS_HANDLE"
	tst_brkm TBROK "unable to create a new network namespace"
fi
TST_CLEANUP=cleanup
ls /sys/class/net >sysfs_before


ns_exec $NS_HANDLE ip link add dummy0 type dummy || \
	tst_brkm TBROK "failed to add a new dummy device"
ns_exec $NS_HANDLE mount -t sysfs none /sys 2>/dev/null

ns_exec $NS_HANDLE test -d /sys/class/net/dummy0
if [ $? -eq 0 ]; then
	tst_resm TPASS "sysfs in new namespace has dummy0 interface"
else
	tst_resm TFAIL "sysfs in new namespace does not have dummy0 interface"
fi

ls /sys/class/net >sysfs_after
diff sysfs_before sysfs_after
if [ $? -eq 0 ]; then
	tst_resm TPASS "sysfs not affected by a separate namespace"
else
	tst_resm TFAIL "sysfs affected by a separate namespace"
fi

tst_exit
