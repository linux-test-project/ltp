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
# The sleep process is frozen. We then move the sleep process to a THAWED
# cgroup. We expect moving the sleep process to fail. [While developing
# the cgroup freezer it was undecided whether moving would fail or it would
# cause the frozen task to thaw. If it causes the task to thaw then replace the
# previous sentence with "We expect the moving task to thaw.". This note can
# be removed once the cgroup freezer is in mainline. ]
#

. "${CGROUPS_TESTROOT}/libcgroup_freezer"
SETS_DEFAULTS="${TCID=freeze_move_thaw.sh} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

running_cgroup_test
mount_freezer && {
make_sample_cgroup_named "child2" && {
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

	# MOVE
	"${CG_FILE_WRITE}" $sample_proc > "$(dirname $(pwd))/child2/tasks" 2> /dev/null && {
		result=${result:-1}
		/bin/false
	} || {
		/bin/true
	}

	# IF the semantics are: the moved task is unfrozen THEN remove everthing
	# after "2> /dev/null" in the line above (don't redirect stdder and
	# don't invert the result)
	# This block of comments can be removed once the cgroup freezer has
	# been added to mainline.

	sleep 1

	assert_sample_proc_is_frozen
	assert_sample_proc_in_cgroup
	assert_sample_proc_not_in_named_cgroup "$(dirname $(pwd))/child2"
	# IF the semantics are: the moved task is unfrozen THEN replace the
	# asserts above with these and make the THAW section consistent:
		# assert_sample_proc_not_in_cgroup # it's in child2 after all
		# assert_sample_proc_in_named_cgroup "$(dirname $(pwd))/child2"
		# assert_sample_proc_not_frozen
	# This block of comments can be removed once the cgroup freezer has
	# been added to mainline.

	# THAW
	issue_thaw_cmd
	wait_until_thawed
	assert_sample_proc_not_frozen
	assert_sample_proc_in_cgroup
	assert_sample_proc_not_in_named_cgroup "$(dirname $(pwd))/child2"
	# IF the semantics are: the moved task is unfrozen before adding to
	# child2 THEN
		# assert_sample_proc_not_frozen
		# assert_sample_proc_not_in_cgroup
		# assert_sample_proc_in_named_cgroup "$(dirname $(pwd))/child2"
	# This block of comments can be removed once the cgroup freezer has
	# been added to mainline.
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
rm_sample_cgroup_named "child2" ; }
umount_freezer ; }

# Failsafe cleanup
cleanup_freezer || /bin/true

exit $result

