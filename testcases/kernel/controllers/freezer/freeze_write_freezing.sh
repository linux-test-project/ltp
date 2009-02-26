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
# Freeze the cgroup and then make sure that writing "FREEZING" into
# freezer.state reports an error (EIO) and doesn't change the freezer's state
# (which was "FROZEN").
#

. "${CGROUPS_TESTROOT}/libcgroup_freezer"
SETS_DEFAULTS="${TCID=freeze_write_freezing.sh} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

running_cgroup_test
mount_freezer && {
make_sample_cgroup && {

while /bin/true ; do
	trap 'break' ERR
	assert_cgroup_freezer_state "THAWED" \
			"ERROR: cgroup freezer started in non-THAWED state"
	issue_freeze_cmd
	wait_until_frozen
	"${CG_FILE_WRITE}" "FREEZING" > freezer.state 2>&1 | \
		grep -E 'Input/output error$'
	assert_cgroup_freezer_state "FROZEN" "ERROR: writing FREEZING to freezer.state should not change freezer state from FROZEN (expected IO error)"
	result=$FINISHED
	break
done
trap '' ERR
cleanup_cgroup_test
tst_resm TINFO " Cleaning up $0"

rm_sample_cgroup ; }
umount_freezer ; }

# Failsafe cleanup
cleanup_freezer || /bin/true

exit $result
