#!/bin/bash

# Copyright (c) International Business Machines  Corp., 2009
# Author: Matt Helsley <matthltc@us.ibm.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

#
# This bash script tests freezer code by starting a long sleep process.
# The sleep process is frozen. We then thaw the process before it exits.
# We expect the process to still be alive as we cleanup the test.
#

SETS_DEFAULTS="${CGROUPS_TESTROOT=..} ${TCID=fork_freeze.sh} ${TST_COUNT=1} ${TST_TOTAL=1}"
. "${CGROUPS_TESTROOT}/libcgroup_freezer"
. "${CGROUPS_TESTROOT}/libltp"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

running_cgroup_test
mount_freezer && {
make_sample_cgroup && {
start_sample_proc "${CGROUPS_TESTROOT}/timed_forkbomb" 4 && {

while /bin/true ; do
	trap 'break' ERR
	assert_cgroup_freezer_state "THAWED" \
			"ERROR: cgroup freezer started in non-THAWED state"
	add_sample_proc_to_cgroup
	echo "go" >> /proc/$sample_proc/fd/0
	sleep "0.5s"
	issue_freeze_cmd
	wait_until_frozen
	assert_sample_proc_is_frozen

	issue_thaw_cmd
	wait_until_thawed
	assert_sample_proc_not_frozen

	sleep 4

	result=$FINISHED
	break
done
trap '' ERR
cleanup_cgroup_test
tst_resm TINFO " Cleaning up $0"

# We need to kill the sample process.
kill_sample_proc ; }
rm_sample_cgroup ; }
umount_freezer ; }

# Failsafe cleanup
cleanup_freezer || /bin/true

exit $result
