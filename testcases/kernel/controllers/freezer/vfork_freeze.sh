#!/bin/sh

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
# This bash script tests freezer code by starting a process with vfork(2).
# vfork causes the freezer to wait until the vfork call "returns" to the
# parent.
#

# we need the vfork test binary -- ensure it's been built
CGROUPS_TESTROOT=${CGROUPS_TESTROOT:=$(dirname "$0")}

if [ ! -x "$CGROUPS_TESTROOT/vfork" ] ; then

	print_make_message=1

	# Maintain ease-of-use backwards compatibility so Matt doesn't want to
	# hang me for the script change :].
	if type make > /dev/null ; then
		make all && print_make_message=0
	fi

	if [ $print_make_message -eq 1 ] ; then
		cat <<EOF
${0##*/}: ERROR: you must run \`make all' in $CGROUPS_TESTROOT before running
this script.
EOF
		exit 1
	fi
fi

. "${CGROUPS_TESTROOT}/libcgroup_freezer"
SETS_DEFAULTS="${TCID=vfork_freeze.sh} ${TST_COUNT=1} ${TST_TOTAL=1}"
declare -r TCID
declare -r TST_COUNT
declare -r TST_TOTAL
export TCID TST_COUNT TST_TOTAL

TMPDIR=${TMPDIR:=/tmp}
TMPLOG="$TMPDIR/${0##*/}.$$.txt"

# We replace the normal sample process with a process which uses vfork to
# create new processes. The vfork'ed processes then sleep, causing the
# parent process ($sample_proc) to enter the TASK_UNINTERRUPTIBLE state
# for the duration of the sleep.
vfork_sleep()
{
	vfork -s$sample_sleep 1 -f "$TMPLOG" &
	local rc=$?
	export vfork_proc=$!
	read sample_proc < "$TMPLOG"
	rm -f "$TMPLOG"
	export sample_proc

	return $rc
}

running_cgroup_test
mount_freezer && {
make_sample_cgroup && {
assert_cgroup_freezer_state "THAWED" \
		"ERROR: cgroup freezer started in non-THAWED state" && {

vfork_sleep && {

while [ 1 ] ; do
	trap 'break' ERR

	add_sample_proc_to_cgroup
	"${CG_FILE_WRITE}" $vfork_proc >> tasks # should add to the same cgroup as above

	issue_freeze_cmd
	wait_until_frozen
	assert_sample_proc_is_frozen
	assert_task_is_frozen $vfork_proc

	issue_thaw_cmd
	wait_until_thawed
	assert_sample_proc_not_frozen
	assert_task_not_frozen $vfork_proc

	result=$FINISHED
	break
done
trap '' ERR
cleanup_cgroup_test
tst_resm TINFO " Cleaning up $0"

# We need to kill the sample process(es).
kill_sample_proc ; export sample_proc=$vfork_proc ; kill_sample_proc ; }

# no inverse op needed for assert
}

rm_sample_cgroup ; }
umount_freezer ; }

rm -f "$TMPLOG"

# Failsafe cleanup
cleanup_freezer || /bin/true

exit $result
