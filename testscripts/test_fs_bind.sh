#!/bin/bash

#
# Copyright (c) International Business Machines  Corp., 2005
# Authors: Avantika Mathur (mathurav@us.ibm.com)
#          Matt Helsley (matthltc@us.ibm.com)
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

if tst_kvcmp -lt "2.6.15"; then
       tst_resm TCONF "System kernel version is less than 2.6.15"
       tst_resm TCONF "Cannot execute test"
       exit 0
fi

test_setup()
{
	#######################################################################
	## Configure
	#######################################################################
	dopts='-dEBb'

	## Remove logged test state depending on results. 0 means do not remove,
	## 1 means OK to remove.
	# rm saved state from tests that appear to have cleaned up properly?
	rm_ok=1
	# rm saved state from tests that don't appear to have fully cleaned up?
	rm_err=0

	#######################################################################
	## Initialize some variables
	#######################################################################
	TCID="$0"
	TST_COUNT=0

	test_dirs=( move bind rbind regression )  #cloneNS
	nfailed=0
	nsucceeded=0

	# set the LTPROOT directory
	cd `dirname $0`
	LTPROOT="${PWD}"
	echo "${LTPROOT}" | grep testscripts > /dev/null 2>&1
	if [ $? -eq 0 ]; then
		cd ..
		LTPROOT="${PWD}"
	fi

	FS_BIND_ROOT="${LTPROOT}/testcases/bin/fs_bind"

	total=0 # total number of tests
	for dir in "${test_dirs[@]}" ; do
		((total += `ls "${FS_BIND_ROOT}/${dir}/test"* | wc -l`))
	done
	TST_TOTAL=${total}

	# set the PATH to include testcases/bin
	LTPBIN="${LTPROOT}/testcases/bin"
	PATH="${PATH}:/usr/sbin:${LTPBIN}:${FS_BIND_ROOT}/bin"

	# Results directory
	resdir="${LTPROOT}/results/fs_bind"
	if [ ! -d "${resdir}" ]; then
		mkdir -p "${resdir}" 2> /dev/null
	fi

	TMPDIR="${TMPDIR:-/tmp}"
	# A temporary directory where we can do stuff and that is
	# safe to remove
	sandbox="${TMPDIR}/sandbox"

	ERR_MSG=""

	export LTPBIN PATH FS_BIND_ROOT ERR_MSG TCID TST_COUNT TST_TOTAL

	if [ ! -d "${resdir}" ]; then
		tst_brkm TBROK true "$0: failed to make results directory"
		exit 1
	fi
}

test_prereqs()
{
	# Must be root to run the containers testsuite
	if [ $UID != 0 ]; then
		tst_brkm TBROK true "FAILED: Must be root to execute this script"
		exit 1
	fi

	mkdir "${sandbox}" >& /dev/null
	if [ ! -d "${sandbox}" -o ! -x "${sandbox}" ]; then
		tst_brkm TBROK true "$0: failed to make directory \"${sandbox}\""
		exit -1
	fi

	mount --bind "${sandbox}" "${sandbox}" >& /dev/null
	if [ $? -ne 0 ]; then
		tst_brkm TBROK true "$0: failed to perform bind mount on directory \"${sandbox}\""
		exit 1
	fi

	mount --make-private "${sandbox}" >& /dev/null
	if [ $? -ne 0 ]; then
		tst_brkm TBROK true "$0: failed to make private mountpoint on directory \"${sandbox}\""
		exit 1
	fi

	local mnt_bind=1
	local mnt_move=1

	pushd "${sandbox}" > /dev/null && {
		mkdir bind_test move_test && {
			mount --bind bind_test bind_test && {
				mnt_bind=0
				mount --move bind_test move_test && {
					mnt_move=0
					umount move_test
				} || {
					# bind mount succeeded but move mount
					# failed
					umount bind_test
				}
			} || {
				# mount failed -- check if it's because we
				# don't have privileges we need
				if [ $? -eq 32 ]; then
					tst_brkm TBROK true "$0 requires the privilege to use the mount command"
					exit 32
				fi
			}
			rmdir bind_test move_test
		}
		popd > /dev/null
	}

	if [ ${mnt_bind} -eq 1 -o ${mnt_move} -eq 1 ]; then
		tst_brkm TBROK true "$0: requires that mount support the --bind and --move options"
		exit 1
	fi

	if tst_kvcmp -lt "2.6.15"; then
		tst_resm TWARN "$0: the remaining tests require 2.6.15 or later"
		tst_exit 0
		exit
	else
		tst_resm TINFO "$0: kernel >= 2.6.15 detected -- continuing"
	fi

	mount --make-shared "${sandbox}" > /dev/null 2>&1 || "${FS_BIND_ROOT}/bin/smount" "${sandbox}" shared
	umount "${sandbox}" || {
		tst_resm TFAIL "$0: failed to umount simplest shared subtree"
		exit 1
	}
	tst_resm TPASS "$0: umounted simplest shared subtree"

}

# mounts we are concerned with in a well-defined order (helps diff)
# returns grep return codes
grep_proc_mounts()
{
	local rc=0

	# Save the pipefail shell option
	shopt -o -q pipefail
	local save=$?
	set -o pipefail

	# Grep /proc/mounts which is often more up-to-date than mounts
	# We use pipefail because if the grep fails we want to pass that along
	grep -F "${sandbox}" /proc/mounts | sort -b
	rc=$?

	# Restore the pipefail shell options
	[ $save -eq 0 ] && shopt -o -s pipefail || shopt -o -u pipefail

	return $rc
}

# Record the mount state
save_proc_mounts()
{
	touch "$2/proc_mounts.before" >& /dev/null
	if [ $? -ne 0 ]; then
		tst_brkm TBROK true "$1: failed to record proc mounts"
		return 1
	fi

	grep_proc_mounts 2> /dev/null > "$2/proc_mounts.before"
	return 0
}

# Compare mount list after the test with the list from before.
# If there are no differences then remove the before list and silently
# return 0. Else print the differences to stderr and return 1.
check_proc_mounts()
{
	local tname="$1"

	if [ ! -r "$2/proc_mounts.before" ]; then
		tst_brkm TBROK true "${tname}: Could not find pre-test proc mount list"
		return 1
	fi

	grep_proc_mounts 2> /dev/null > "$2/proc_mounts.after"
	# If the mounts are the same then just return
	diff ${dopts} -q "$2/proc_mounts.before" "$2/proc_mounts.after" >& /dev/null
	if [ $? -eq 0 ]; then
		[ $rm_ok -eq 1 ] && rm -f "$2/proc_mounts."{before,after}
		return 0
	fi

	tst_resm TWARN "${tname}: did not properly clean up its proc mounts"
	diff ${dopts} -U 0 "$2/proc_mounts.before" "$2/proc_mounts.after" | grep -vE '^\@\@' 1>&2
	[ $rm_err -eq 1 ] && rm -f "$2/proc_mounts."{before,after}
	return 1
}

# Undo leftover mounts
restore_proc_mounts()
{
	#local tname="$1"

	# do lazy umounts -- we're assuming that tests will only leave
	# new mounts around and will never remove mounts outside the test
	# directory
	( while grep_proc_mounts ; do
		grep_proc_mounts | awk '{print $2}' | xargs -r -n 1 umount -l
	done ) >& /dev/null

	# mount list and exit with 0
	[ $rm_err -eq 1 ] && rm -f "$2/proc_mounts."{before,after} 1>&2 # >& /dev/null
	return 0
	# if returning error do this:
	# tst_brkm TBROK true "${tname}: failed to restore mounts"
}

# mounts we are concerned with in a well-defined order (helps diff)
# returns grep return codes
grep_mounts()
{
	local rc=0

	# Save the pipefail shell option
	shopt -o -q pipefail
	local save=$?
	set -o pipefail

	# Grep mount command output (which tends to come from /etc/mtab)
	# We use pipefail because if the grep fails we want to pass that along
	mount | grep -F "${sandbox}" | sort -b
	rc=$?

	# Restore the pipefail shell options
	[ $save -eq 0 ] && shopt -o -s pipefail || shopt -o -u pipefail

	return $rc
}

# Record the mount state
save_mounts()
{
	touch "$2/mtab.before" >& /dev/null
	if [ $? -ne 0 ]; then
		tst_brkm TBROK true "$1: failed to record mtab mounts"
		return 1
	fi

	grep_mounts 2> /dev/null > "$2/mtab.before"
	return 0
}

# Compare mount list after the test with the list from before.
# If there are no differences then remove the before list and silently
# return 0. Else print the differences to stderr and return 1.
check_mounts()
{
	local tname="$1"

	if [ ! -r "$2/mtab.before" ]; then
		tst_brkm TBROK true "${tname}: Could not find pre-test mtab mount list"
		return 1
	fi

	grep_mounts 2> /dev/null > "$2/mtab.after"
	# If the mounts are the same then just return
	diff ${dopts} -q "$2/mtab.before" "$2/mtab.after" >& /dev/null
	if [ $? -eq 0 ]; then
		[ $rm_ok -eq 1 ] && rm -f "$2/mtab."{before,after}
		return 0
	fi

	tst_resm TWARN "${tname}: did not properly clean up its mtab mounts"
	diff ${dopts} -U 0 "$2/mtab.before" "$2/mtab.after" | grep -vE '^\@\@' 1>&2
	[ $rm_err -eq 1 ] && rm -f "$2/mtab."{before,after}
	return 1
}

# Undo leftover mounts
restore_mounts()
{
	#local tname="$1"

	# do lazy umounts -- we're assuming that tests will only leave
	# new mounts around and will never remove mounts outside the test
	# directory
	( while grep_mounts ; do
		grep_mounts | awk '{print $3}' | xargs -r -n 1 umount -l
	done ) >& /dev/null

	# mount list and exit with 0
	[ $rm_err -eq 1 ] && rm -f "$2/mtab."{before,after} 1>&2 # >& /dev/null
	return 0
	# if returning error do this:
	# tst_brkm TBROK true "${tname}: failed to restore mounts"
}

# Record the sandbox state
# We don't save full sandbox state -- just the names of files and dirs present
save_sandbox()
{
	local when="before"
	local tname="$1"

	if [ -e "$2/files.before" ]; then
		if [ -e "$2/files.after" ]; then
			tst_brkm TBROK true "${tname}: stale catalog of \"${sandbox}\""
			return 1
		fi
		when="after"
	fi

	( find "${sandbox}" -type d -print | sort > "$2/dirs.$when"
	  find "${sandbox}" -type f -print | sort | \
		grep -vE '^'"$2"'/(dirs|files)\.(before|after)$' > "$2/files.$when" ) >& /dev/null
	return 0
}

# Save sandbox after test and then compare. If the sandbox state is not
# clean then print the differences to stderr and return 1. Else remove all
# saved sandbox state and silently return 0
check_sandbox()
{
	local tname="$1"

	if [ ! -r "$2/files.before" -o ! -r "$2/dirs.before" ]; then
		tst_brkm TBROK true "${tname} missing saved catalog of \"${sandbox}\""
		return 1
	fi

	save_sandbox "${tname} (check)" "$2"

	( diff ${dopts} -q "$2/dirs.before" "$2/dirs.after" && \
	  diff ${dopts} -q "$2/files.before" "$2/files.after" )  >& /dev/null \
	  && {
		[ $rm_ok -eq 1 ] && rm -f "$2/"{files,dirs}.{before,after}
		return 0
	}

	tst_resm TWARN "${tname} did not properly clean up \"${sandbox}\""
	diff ${dopts} -U 0 "$2/dirs.before" "$2/dirs.after" 1>&2
	diff ${dopts} -U 0 "$2/files.before" "$2/files.after" 1>&2
	[ $rm_err -eq 1 ] && rm -f "$2/"{files,dirs}.{before,after} 1>&2
	return 1
}

# Robust sandbox cleanup
clean_sandbox()
{
	local tname="$1"

	{ rm -rf "${sandbox}" ; mkdir "${sandbox}" ; } >& /dev/null
	if [ ! -d "${sandbox}" -o ! -x "${sandbox}" ]; then
		tst_brkm TBROK true "$tname: failed to make directory \"${sandbox}\""
		return 1
	fi
	return 0
}

# Check file for non-whitespace chars
is_file_empty()
{
	awk '/^[[:space:]]*$/  { next }
	      { exit 1; }' < "$1"
}

#
# Run the specified test script.
#
# Return 1 if the test was broken but should not stop the remaining test
#	categories from being run.
# Return 2 if the test was broken and no further tests should be run.
# Return 0 otherwise (if the test was broken but it shouldn't affect other
#	test runs)
# Note that this means the return status is not the success or failure of the
#	test itself.
#
run_test()
{
	local t="$1"
	local tname="$(basename "$(dirname "$t")")/$(basename "$t")"
	local log="$resdir/$tname/log"
	local errlog="$resdir/$tname/err"
	local do_break=0

	ERR_MSG=""

	# Pre-test
	mkdir -p "$resdir/$tname"
	if [ ! -d "$resdir/$tname" -o ! -x "$resdir/$tname" ]; then
		tst_brkm TBROK true "$0: can't make or use \"$resdir/$tname\" as a log directory"
		return 1
	fi

	save_sandbox "$tname" "$resdir/$tname" || do_break=1
	save_mounts "$tname" "$resdir/$tname" || do_break=1
	save_proc_mounts "$tname" "$resdir/$tname" || do_break=1
	mount --bind "${sandbox}" "${sandbox}" >& /dev/null || do_break=1
	mount --make-private "${sandbox}" >& /dev/null || do_break=1

	if [ $do_break -eq 1 ]; then
		umount -l "${sandbox}" >& /dev/null
		tst_brkm TBROK true "$tname: failed to save pre-test state of \"${sandbox}\""
		return 2
	fi
	pushd "${sandbox}" > /dev/null

	# Run the test
	(
		TCID="$tname"
		declare -r TST_COUNT
		export LTPBIN PATH FS_BIND_ROOT ERR_MSG TCID TST_COUNT TST_TOTAL
		"$t" #> "$log" 2> "$errlog"
	)
	local rc=$?
	TCID="$0"

	# Post-test
	popd > /dev/null
	if [ $rc -ne 0 ]; then
		#echo "FAILED"
		((nfailed++))
	else
		#echo "SUCCEEDED"
		((nsucceeded++))
	fi
	umount -l "${sandbox}" >& /dev/null
	check_proc_mounts "$tname" "$resdir/$tname" || \
	restore_proc_mounts "$tname" "$resdir/$tname" || do_break=1
	check_mounts "$tname" "$resdir/$tname" || \
	restore_mounts "$tname" "$resdir/$tname" || do_break=1
	check_sandbox "$tname" "$resdir/$tname"
	clean_sandbox "$tname" || do_break=1
	if [ $do_break -eq 1 ]; then
		tst_brkm TBROK true "$tname: failed to restore pre-test state of \"${sandbox}\""
		return 2
	fi

	# If we succeeded and the error log is empty remove it
	if [ $rc -eq 0 -a -w "$errlog" ] && is_file_empty "$errlog" ; then
		rm -f "$errlog"
	fi
	return 0
}

main()
{
	TST_COUNT=1
	for dir in "${test_dirs[@]}" ; do
		tests=( $(find "${FS_BIND_ROOT}/${dir}" -type f -name 'test*') )
		clean_sandbox "$0" || break
		for t in "${tests[@]}" ; do
			run_test "$t"
			local rc=$?

			if [ $rc -ne 0 ]; then
				break $rc
			fi

			((TST_COUNT++))
		done
	done
	rm -rf "${sandbox}"
	return 0

	skipped=$((total - nsucceeded - nfailed))
	if [ $nfailed -eq 0 -a $skipped -eq 0 ]; then
		# Use PASSED for the summary rather than SUCCEEDED to make it
		# easy to determine 100% success from a calling script
		summary="PASSED"
	else
		# Use FAILED to make it easy to find > 0% failure from a
		# calling script
		summary="FAILED"
	fi
	cat - <<-EOF
		*********************************
		RESULTS SUMMARY:

			passed:  $nsucceeded/$total
			failed:  $nfailed/$total
			skipped: $skipped/$total
			summary: $summary

		*********************************
	EOF
}

test_setup || exit 1
test_prereqs || exit 1
declare -r FS_BIND_ROOT
declare -r TST_TOTAL
main  #2> "$resdir/errors" 1> "$resdir/summary"
