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
# The sleep process is stopped and then frozen. We then thaw the process
# after it normally would have exited.
# We expect the process to still be around as we cleanup the test.
#

. "${CGROUPS_TESTROOT}/libcgroup_freezer"
SETS_DEFAULTS="${TCID=stop_freeze_sleep_thaw_cont.sh} ${TST_COUNT=1} ${TST_TOTAL=1}"
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

	kill -SIGSTOP $sample_proc
	# TODO: reliably fix race between signal and assertion?
	/bin/sleep 1
	assert_sample_proc_stopped

	issue_freeze_cmd
	wait_until_frozen
	assert_sample_proc_is_frozen

	echo -n "INFO: Waiting until sleep command should have finished ($sample_proc) ... "
	/bin/sleep $(($sample_sleep * 2))
	echo "done."

	issue_thaw_cmd
	wait_until_thawed
	assert_sample_proc_exists
	assert_sample_proc_not_frozen
	assert_sample_proc_stopped

	kill -SIGCONT $sample_proc
	# TODO: reliably fix race between signal and assertion?
	/bin/sleep 1
	assert_sample_proc_does_not_exist

	# We don't need to kill the sample process.
	sample_proc=""
	result=$FINISHED
	break
done
trap '' ERR
cleanup_cgroup_test
tst_resm TINFO " Cleaning up $0"

# We may not need to kill the sample process.
kill_sample_proc ; }
rm_sample_cgroup ; }
umount_freezer ; }

# Failsafe cleanup
cleanup_freezer || /bin/true

exit $result
