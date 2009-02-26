#!/bin/bash

# Copyright (c) International Business Machines  Corp., 2008
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
# The sleep process is frozen. We then kill the sleep process.
# Then we unfreeze the sleep process and see what happens. We expect the
# sleep process to receive the kill signals and exit almost immediately
# after the cgroup is thawed.
#

. "${CGROUPS_TESTROOT}/libcgroup_freezer"
SETS_DEFAULTS="${TCID=freeze_kill_thaw.sh} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

running_cgroup_test
mount_freezer && {
make_sample_cgroup && {
start_sample_proc && {

while /bin/true ; do
	trap 'break' ERR
	assert_cgroup_freezer_state "THAWED" \
			"ERROR: cgroup freezer started in non-THAWED state"
	add_sample_proc_to_cgroup

	# FREEZE
	issue_freeze_cmd
	wait_until_frozen
	assert_sample_proc_is_frozen

	# KILL
	# NOTE: not "send_signal" because we're in the freezer tests
	echo -n "INFO: Killing $sample_proc ..."
	kill $sample_proc || kill -9 $sample_proc
	echo " done."

	# THAW
	# We expect that kill will not complete until the sample process is
	# thawed. We also expect the task to remain visible to 'ps'.
	# TODO: reliably fix race between signal and assert ??
	assert_sample_proc_is_frozen

	issue_thaw_cmd
	wait_until_thawed

	# We do NOT call
	#      assert_task_not_frozen $sample_proc
	# here because we've killed the sample process.
	assert_sample_proc_does_not_exist
	sample_proc=""
	result=$FINISHED
	break
done
trap '' ERR
cleanup_cgroup_test
tst_resm TINFO " Cleaning up $0"

# We may need to stop the sample process because we could have failed before the
# rest of the test caused it to stop.
kill_sample_proc ; }
rm_sample_cgroup ; }
umount_freezer ; }

# Failsafe cleanup
cleanup_freezer || /bin/true

exit $result

