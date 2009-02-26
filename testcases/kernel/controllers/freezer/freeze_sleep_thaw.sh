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
# The sleep process is frozen. We then wait until the sleep process should
# have exited. Then we unfreeze the sleep process. We expect the
# sleep process to wakeup almost immediately after the cgroup is thawed,
# recognize that its expiration time has long since passed, and exit before
# we get a chance to "see" it again.
#

. "${CGROUPS_TESTROOT}/libcgroup_freezer"
SETS_DEFAULTS="${TCID=freeze_sleep_thaw.sh} ${TST_COUNT=1} ${TST_TOTAL=1}"
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
	add_sample_proc_to_cgroup
	assert_cgroup_freezer_state "THAWED" \
			"ERROR: cgroup freezer started in non-THAWED state"
	issue_freeze_cmd
	wait_until_frozen
	assert_sample_proc_is_frozen

	echo -n "INFO: Waiting until sleep command should have finished ($sample_proc) ... "
	/bin/sleep $(($sample_sleep * 2))
	echo "done."

	# We expect the task to remain until it is thawed even though its
	# sleep period has long since expired.
	assert_sample_proc_is_frozen

	issue_thaw_cmd
	wait_until_thawed

	# We do NOT call
	#      assert_task_not_frozen $sample_proc
	# here because we've slept past the point in time when the task should've
	# exited.
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
