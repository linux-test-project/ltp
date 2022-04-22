#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2005
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Based on work by: Avantika Mathur (mathurav@us.ibm.com)

TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_MIN_KVER=2.6.15
TST_SETUP="${TST_SETUP:-fs_bind_setup}"
TST_CLEANUP="${TST_CLEANUP:-fs_bind_cleanup}"
TST_TESTFUNC=fs_bind_test
TST_NEEDS_CMDS="mount umount awk sed"

. tst_test.sh

[ -z "$FS_BIND_TESTFUNC" ] && tst_brk TBROK "Please set FS_BIND_TESTFUNC before sourcing fs_bind_lib.sh"

# Test interface:
#
# FS_BIND_TESTFUNC is the real testfunction. Please do not use
# TST_TESTFUNC in the test. FS_BIND_TESTFUNC is used to wrap some additional logic.
# TST_CNT can be used as usual

FS_BIND_SANDBOX="sandbox"
FS_BIND_DISK1="disk1"
FS_BIND_DISK2="disk2"
FS_BIND_DISK3="disk3"
FS_BIND_DISK4="disk4"

FS_BIND_MNTNS_PID=

# Creates a directory and bind-mounts it to itself.
# usage: fs_bind_makedir share_mode directory
# where
#  share_mode is one of the --make-{shared,priv,...}
#  supported by mount
#  directory is directory where to be created/configured.
#  If it does not exist, it will be created
fs_bind_makedir()
{
	local bind_type dir

	bind_type="$1"
	dir="$2"

	case "$bind_type" in
	shared|private|rshared|slave|rslave|rprivate|runbindable) ;;
	*) tst_brk TBROK "Unknown share type \"$bind_type\""
	esac

	if [ -e "$dir" ]; then
		tst_brk TBROK "An entry by the name \"$dir\" exists already"
	fi

	ROD mkdir -p "$dir"

	# Most mount implementations can use --make-* in the same command as bind,
	# but busybox mount, fails at least to set --make-private
	EXPECT_PASS mount --bind "$dir" "$dir"
	EXPECT_PASS mount --make-$bind_type "$dir"
}

# Verifies that subtrees contain the same content
# usage: fs_bind_check [-n] dir1 dirn...
# where
#  -n  If set, expectes the subtrees to not be equal
#  -s  Run check in mount namespace
# dir1 dirn... An arbitraty number of directories, that are compared.
#              If -n is given, only two directories are allowed.
fs_bind_check()
{
	local OPTIND expect_diff use_ns args msg dir1 dir2 fail output
	expect_diff=0
	use_ns=0
	while getopts "ns" args; do
		case "$args" in
		n) expect_diff=1; ;;
		s) use_ns=1; ;;
		esac
	done
	shift $((OPTIND-1))
	msg="Check"
	[ $expect_diff -eq 1 ] && msg="$msg no"
	msg="$msg propagation"
	if [ $use_ns -eq 1 ]; then
		[ -z "$FS_BIND_MNTNS_PID" ] && tst_brk TBROK "Namespace does not exist"
		msg="$msg in mnt namespace"
	fi
	msg="$msg $*"

	if [ $# -lt 2 ] || ( [ $expect_diff -eq 1 ] && [ $# -ne 2 ] ); then
		tst_brk TBROK "Insufficient arguments"
	fi

	dir1=$1
	shift

	for dir2 in "$@"; do
		# compare adjacent pairs of directory trees

	    if [ ! -d "$dir1" ]; then
	        tst_res TFAIL "$msg: \"$dir1\" does not exist"
	        return 1
	    elif [ ! -d "$dir2" ]; then
	        if [ $expect_diff -eq 1 ]; then
	            tst_res TPASS "$msg"
	            return 0
	        else
	            tst_res TFAIL "$msg: \"$dir2\" does not exist"
	            return 1
	        fi
	    fi

		if [ $use_ns -eq 1 ]; then
			output="$(ns_exec ${FS_BIND_MNTNS_PID} mnt diff -r "$PWD/$dir1" "$PWD/$dir2" 2> /dev/null)"
		else
			output="$(diff -r "$dir1" "$dir2" 2> /dev/null)"
		fi

		if [ $? -ne 0 ]; then
			if [ $expect_diff -eq 1 ]; then
	            # Since expect_diff=1 allows only two directories
	            # to be compared, we are done after finding the first diff
				tst_res TPASS "$msg"
				return 0
			else
				tst_res TFAIL "$msg:"
	            tst_res TFAIL "\"$dir1\" \"$dir2\" differ:\n$output"
				return 1
			fi
		fi
		dir1="$dir2"
	done

	if [ $expect_diff -eq 1 ]; then
		tst_res TFAIL "$msg"
		return 1
	else
		tst_res TPASS "$msg"
		return 0
	fi
}

fs_bind_setup()
{
	fs_bind_makedir private "$FS_BIND_SANDBOX"

	cd $FS_BIND_SANDBOX
	mkdir -p "$FS_BIND_DISK1" "$FS_BIND_DISK2" "$FS_BIND_DISK3" "$FS_BIND_DISK4"
	mkdir "$FS_BIND_DISK1/a" "$FS_BIND_DISK1/b" "$FS_BIND_DISK1/c"
	mkdir "$FS_BIND_DISK2/d" "$FS_BIND_DISK2/e" "$FS_BIND_DISK2/f"
	mkdir "$FS_BIND_DISK3/g" "$FS_BIND_DISK3/h" "$FS_BIND_DISK3/i"
	mkdir "$FS_BIND_DISK4/j" "$FS_BIND_DISK4/k" "$FS_BIND_DISK4/l"
}

_fs_bind_unmount_all()
{
	local mounts

	cd "$TST_TMPDIR"

	# Cleanup leftover mounts from the test
	# sed '1!G;h;$!d' is used to reverse /proc/mounts.
	# unmounting in reverse order requires significantly less iterations
	# There is a slight chance, this loop does not terminate, if a mount
	# cannot be unmounted for some reason. In that case the timeout
	# will kill the test, but we cannot restore the system anyway
	while true; do
		mounts=$( sed '1!G;h;$!d' /proc/mounts \
			| awk -vtmp="$TST_TMPDIR/$FS_BIND_SANDBOX/" \
			'index($2, tmp) {print $2}' )
		[ -z "$mounts" ] && break
		echo $mounts | xargs umount 2>/dev/null
	done
}

fs_bind_cleanup()
{
	_fs_bind_unmount_all
	umount "$FS_BIND_SANDBOX"
}

_fs_bind_setup_test()
{
	local e

	cd "$TST_TMPDIR/$FS_BIND_SANDBOX" || tst_brk "Unable to cd into sandbox"

	for e in ls *; do
		if   [ "$e" = "$FS_BIND_DISK1" ] \
		  || [ "$e" = "$FS_BIND_DISK2" ] \
		  || [ "$e" = "$FS_BIND_DISK3" ] \
		  || [ "$e" = "$FS_BIND_DISK4" ]; then
		  continue
		fi
		rm -rf "$e"
	done
}

fs_bind_create_ns()
{
	[ -n "$FS_BIND_MNTNS_PID" ] && tst_brk TBROK "Namespace exist already"
	FS_BIND_MNTNS_PID=$(ns_create mnt)
}

fs_bind_exec_ns()
{
	[ -z "$FS_BIND_MNTNS_PID" ] && tst_brk TBROK "Namespace does not exist"
	EXPECT_PASS ns_exec $FS_BIND_MNTNS_PID mnt "$@"
}

fs_bind_destroy_ns()
{
	[ -n "$FS_BIND_MNTNS_PID" ] && kill $FS_BIND_MNTNS_PID 2>/dev/null
	FS_BIND_MNTNS_PID=
}

_fs_bind_cleanup_test()
{
	local mounts

	fs_bind_destroy_ns

	mounts=$( awk -v tmp="$TST_TMPDIR/$FS_BIND_SANDBOX/" '
		index($2, tmp) {
			print substr($2, length(tmp) + 1)
		}
	' /proc/mounts )
	if [ -n "$mounts" ]; then
		tst_res TFAIL "There are still mounts in the sandbox:\n$mounts"
	fi
	_fs_bind_unmount_all
}

fs_bind_test()
{
	_fs_bind_setup_test

	if type ${FS_BIND_TESTFUNC}1 > /dev/null 2>&1; then
		"$FS_BIND_TESTFUNC$_tst_i" $1
	else
		"$FS_BIND_TESTFUNC" $1
	fi

	_fs_bind_cleanup_test
}
