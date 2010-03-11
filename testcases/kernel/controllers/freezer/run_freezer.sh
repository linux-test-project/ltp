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

#set -x
P='ltp-cgroup-freezer'

export EXIT_GOOD=0
export EXIT_TERMINATE=-1


trap "exit ${EXIT_TERMINATE}" ERR

#
# Run tests for freezer and signal controllers
#

	# TODO add:
	#vfork_freeze.sh
	# write signal/send_invalid_sig.sh
	# write signal/read_signal.kill.sh
FREEZER_TEST_SCRIPTS=(	write_freezing.sh
			freeze_write_freezing.sh
			freeze_thaw.sh
			freeze_sleep_thaw.sh
			freeze_kill_thaw.sh
			freeze_move_thaw.sh
			freeze_cancel.sh
			freeze_self_thaw.sh
			stop_freeze_thaw_cont.sh
			stop_freeze_sleep_thaw_cont.sh
			fork_freeze.sh )

TEST_SCRIPTS=( )

function test_setup()
{
	if [ -z "${LTPROOT}" ]; then
		CGROUPS_TESTROOT="$(pwd)"
		. libltp
	else
		CGROUPS_TESTROOT="${LTPROOT}/testcases/bin"
	fi

	#######################################################################
	## Initialize some LTP variables -- Set _COUNT and _TOTAL to fake values
	## else LTP complains.
	#######################################################################
	TCID="$0"
	TST_COUNT=1
	TST_TOTAL=$(( ${#TEST_SCRIPTS[@]} + 0 ))
	TMPDIR="${TMPDIR:-/tmp}"

	export LTPBIN PATH TCID TST_COUNT TST_TOTAL CGROUPS_TESTROOT
	tst_resm TINFO "Preparing to run: ${P} $@"

	# this is not require here
	#make all
}

function test_prereqs()
{
	cat /proc/filesystems | grep -E '\bcgroup\b' > /dev/null 2>&1 || {
		tst_resm TINFO "Kernel does not support cgroups. Skipping."
		exit ${EXIT_GOOD} # 0
	}

	tst_resm TINFO " Testing prereqs for cgroup freezer tests."
	if [ ! -f /proc/cgroups ]; then
		tst_resm TINFO "Tests require cgroup freezer support in the kernel."
		exit 1
	fi

	if [ "$(grep -w freezer /proc/cgroups | cut -f1)" != "freezer" ]; then
		tst_resm TINFO "Tests require cgroup freezer support in the kernel."
		exit 1
	fi

	. "${CGROUPS_TESTROOT}/libcgroup_freezer"

	tst_resm TINFO " It's ok if there's an ERROR before the next INFO."
	mount_freezer && umount_freezer && {
		TEST_SCRIPTS=( "${TEST_SCRIPTS[@]}" "${FREEZER_TEST_SCRIPTS[@]}" )
	}
	tst_resm TINFO " OK, resume worrying about ERRORS."
	export TST_TOTAL=$(( ${#TEST_SCRIPTS[@]} + 0 ))
}

function main()
{
	local rc=0

	for TEST in "${TEST_SCRIPTS[@]}" ; do
		export TCID="${TEST}"
		tst_resm TINFO " running ${TEST}"
		((TST_COUNT++))
		export TST_COUNT
		"${CGROUPS_TESTROOT}/${TEST}"
		rc=$?

		if [ $rc != 0 ]; then
			tst_resm TFAIL "${TEST} $rc"
			break
		else
			tst_resm TPASS "${TEST}"
		fi
	done
	trap '' ERR
	return $rc
}

test_setup && \
test_prereqs && \
declare -r TST_TOTAL && \
main
rc=$?
trap '' ERR
exit $rc
