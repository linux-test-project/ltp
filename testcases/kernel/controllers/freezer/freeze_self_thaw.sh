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
# This bash script tests freezer code by starting a long subshell process.
# The subshell process sleeps and then freezes the control group it is a
# part of. We then thaw the subshell process. We expect the unthawed subshell
# process to need cleanup afterwards (allows us to test successful thawing).
#

. "${CGROUPS_TESTROOT}/libcgroup_freezer"
SETS_DEFAULTS="${TCID=freeze_self_thaw.sh} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

# We replace the normal sample process with a process which sleeps, issues
# the freeze command, and then sleeps some more. Little does it know that it
# will be freezing the cgroup it's been assigned to.
function sleep_freeze_self_sleep()
{
	( export sample_proc=$! ; \
		add_sample_proc_to_cgroup ; \
		tst_resm TINFO " $sample_proc freezing itself" ; \
		"${CG_FILE_WRITE}" "${FREEZE}" > freezer.state ; \
		tst_resm TINFO " $sample_proc thawed successfully" ; \
		exec sleep $sample_sleep ) &
	local rc=$?
	export sample_proc=$!

	return $rc
}

running_cgroup_test
mount_freezer && {
make_sample_cgroup && {
assert_cgroup_freezer_state "THAWED" \
		"ERROR: cgroup freezer started in non-THAWED state" && {

sleep_freeze_self_sleep && {

while /bin/true ; do
	trap 'break' ERR
	tst_resm TINFO " Waiting for $sample_proc to freeze itself"

	# WAIT until FROZEN (see FREEZE self above)
	wait_until_frozen
	assert_sample_proc_is_frozen

	# THAW
	issue_thaw_cmd
	wait_until_thawed
	assert_sample_proc_exists
	assert_sample_proc_not_frozen

	result=$FINISHED
	break
done
trap '' ERR
cleanup_cgroup_test
tst_resm TINFO " Cleaning up $0"

# We need to stop the sample process.
kill_sample_proc ; }
# no inverse op needed for assert
}
rm_sample_cgroup ; }
umount_freezer ; }

# Failsafe cleanup
cleanup_freezer || /bin/true

exit $result
