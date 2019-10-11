#!/bin/sh
#
# Copyright (c) Linux Test Project, 2014-2017
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Written by Cyril Hrubis <chrubis@suse.cz>
#
# This is a LTP test library for shell.
#

export LTP_RET_VAL=0
export TST_COUNT=1
export TST_PASS_COUNT=0
export TST_LIB_LOADED=1
export TST_TMPDIR_RHOST=0

. tst_ansi_color.sh

# Exit values map
tst_flag2mask()
{
	case "$1" in
	TPASS) return 0;;
	TFAIL) return 1;;
	TBROK) return 2;;
	TWARN) return 4;;
	TINFO) return 16;;
	TCONF) return 32;;
	*) tst_brkm TBROK "Invalid resm type '$1'";;
	esac
}

tst_resm()
{
	local ttype="$1"

	tst_flag2mask "$ttype"
	local mask=$?
	LTP_RET_VAL=$((LTP_RET_VAL|mask))

	local ret=$1
	shift

	printf "$TCID $TST_COUNT "
	tst_print_colored $ret "$ret:"
	echo " $@"

	case "$ret" in
	TPASS|TFAIL|TCONF) TST_COUNT=$((TST_COUNT+1));;
	esac

	if [ "$ret" = TPASS ]; then
		TST_PASS_COUNT=$((TST_PASS_COUNT+1))
	fi
}

tst_brkm()
{
	case "$1" in
	TFAIL) ;;
	TBROK) ;;
	TCONF) ;;
	*) tst_brkm TBROK "Invalid tst_brkm type '$1'";;
	esac

	local ret=$1
	shift
	tst_resm "$ret" "$@"
	tst_exit
}

tst_record_childstatus()
{
	if [ $# -ne 1 ]; then
		tst_brkm TBROK "Requires child pid as parameter"
	fi

	local child_pid=$1
	local ret=0

	wait $child_pid
	ret=$?
	if [ $ret -eq 127 ]; then
		tst_brkm TBROK "Child process pid='$child_pid' does not exist"
	fi
	LTP_RET_VAL=$((LTP_RET_VAL|ret))
}

tst_require_root()
{
	if [ "$(id -ru)" != 0 ]; then
		tst_brkm TCONF "Must be super/root for this test!"
	fi
}

tst_exit()
{
	if [ -n "${TST_CLEANUP:-}" -a -z "${TST_NO_CLEANUP:-}" ]; then
		$TST_CLEANUP
	fi

	if [ -n "${LTP_IPC_PATH:-}" -a -f "${LTP_IPC_PATH:-}" ]; then
		rm -f "$LTP_IPC_PATH"
	fi

	# Mask out TCONF if no TFAIL/TBROK/TWARN but has TPASS
	if [ $((LTP_RET_VAL & 7)) -eq 0 -a $TST_PASS_COUNT -gt 0 ]; then
		LTP_RET_VAL=$((LTP_RET_VAL & ~32))
	fi
	# Mask out TINFO
	exit $((LTP_RET_VAL & ~16))
}

tst_tmpdir()
{
	if [ -z "$TMPDIR" ]; then
		export TMPDIR="/tmp"
	fi

	TST_TMPDIR=$(mktemp -d "$TMPDIR/$TCID.XXXXXXXXXX")

	chmod 777 "$TST_TMPDIR"

	TST_STARTWD=$(pwd)

	cd "$TST_TMPDIR"
}

tst_rmdir()
{
	if [ -n "$TST_TMPDIR" ]; then
		cd "$LTPROOT"
		rm -r "$TST_TMPDIR"
		[ "$TST_TMPDIR_RHOST" = 1 ] && tst_cleanup_rhost
	fi
}

#
# Checks if commands passed as arguments exists
#
tst_require_cmds()
{
	local cmd
	for cmd in $*; do
		if ! command -v $cmd > /dev/null 2>&1; then
			tst_brkm TCONF "'$cmd' not found"
		fi
	done
}

# tst_retry "command" [times]
# try run command for specified times, default is 3.
# Function returns 0 if succeed in RETRIES times or the last retcode the cmd
# returned
tst_retry()
{
	local cmd="$1"
	local RETRIES=${2:-"3"}
	local i=$RETRIES

	while [ $i -gt 0 ]; do
		eval "$cmd"
		ret=$?
		if [ $ret -eq 0 ]; then
			break
		fi
		i=$((i-1))
		sleep 1
	done

	if [ $ret -ne 0 ]; then
		tst_resm TINFO "Failed to execute '$cmd' after $RETRIES retries"
	fi

	return $ret
}

# tst_timeout "command arg1 arg2 ..." timeout
# Runs command for specified timeout (in seconds).
# Function returns retcode of command or 1 if arguments are invalid.
tst_timeout()
{
	local command=$1
	local timeout=$(echo $2 | grep -o "^[0-9]\+$")

	# command must be non-empty string with command to run
	if [ -z "$command" ]; then
		echo "first argument must be non-empty string"
		return 1
	fi

	# accept only numbers as timeout
	if [ -z "$timeout" ]; then
		echo "only numbers as second argument"
		return 1
	fi

	setsid sh -c "eval $command" 2>&1 &
	local pid=$!
	while [ $timeout -gt 0 ]; do
		kill -s 0 $pid 2>/dev/null
		if [ $? -ne 0 ]; then
			break
		fi
		timeout=$((timeout - 1))
		sleep 1
	done

	local ret=0
	if [ $timeout -le 0 ]; then
		ret=128
		kill -TERM -- -$pid
	fi

	wait $pid
	ret=$((ret | $?))

	return $ret
}

ROD_SILENT()
{
	local tst_out="$($@ 2>&1)"
	if [ $? -ne 0 ]; then
		echo "$tst_out"
		tst_brkm TBROK "$@ failed"
	fi
}

ROD_BASE()
{
	local cmd
	local arg
	local file
	local flag

	for arg; do
		file="${arg#\>}"
		if [ "$file" != "$arg" ]; then
			flag=1
			if [ -n "$file" ]; then
				break
			fi
			continue
		fi

		if [ -n "$flag" ]; then
			file="$arg"
			break
		fi

		cmd="$cmd $arg"
	done

	if [ -n "$flag" ]; then
		$cmd > $file
	else
		$@
	fi
}

ROD()
{
	ROD_BASE "$@"
	if [ $? -ne 0 ]; then
		tst_brkm TBROK "$@ failed"
	fi
}

EXPECT_PASS()
{
	ROD_BASE "$@"
	if [ $? -eq 0 ]; then
		tst_resm TPASS "$@ passed as expected"
	else
		tst_resm TFAIL "$@ failed unexpectedly"
	fi
}

EXPECT_FAIL()
{
	# redirect stderr since we expect the command to fail
	ROD_BASE "$@" 2> /dev/null
	if [ $? -ne 0 ]; then
		tst_resm TPASS "$@ failed as expected"
	else
		tst_resm TFAIL "$@ passed unexpectedly"
	fi
}

tst_mkfs()
{
	local fs_type=$1
	local device=$2
	shift 2
	local fs_opts="$@"

	if [ -z "$fs_type" ]; then
		tst_brkm TBROK "No fs_type specified"
	fi

	if [ -z "$device" ]; then
		tst_brkm TBROK "No device specified"
	fi

	tst_resm TINFO "Formatting $device with $fs_type extra opts='$fs_opts'"

	ROD_SILENT mkfs.$fs_type $fs_opts $device
}

tst_umount()
{
	local device="$1"
	local i=0

	if ! grep -q "$device" /proc/mounts; then
		tst_resm TINFO "The $device is not mounted, skipping umount"
		return
	fi

	while [ "$i" -lt 50 ]; do
		if umount "$device" > /dev/null; then
			return
		fi

		i=$((i+1))

		tst_resm TINFO "umount($device) failed, try $i ..."
		tst_resm TINFO "Likely gvfsd-trash is probing newly mounted "\
			       "fs, kill it to speed up tests."

		tst_sleep 100ms
	done

	tst_resm TWARN "Failed to umount($device) after 50 retries"
}

# Check a module file existence
# Should be called after tst_tmpdir()
tst_module_exists()
{
	local mod_name="$1"

	if [ -f "$mod_name" ]; then
		TST_MODPATH="$mod_name"
		return
	fi

	local mod_path="$LTPROOT/testcases/bin/$mod_name"
	if [ -f "$mod_path" ]; then
		TST_MODPATH="$mod_path"
		return
	fi

	if [ -n "$TST_TMPDIR" ]; then
		mod_path="$TST_STARTWD/$mod_name"
		if [ -f "$mod_path" ]; then
			TST_MODPATH="$mod_path"
			return
		fi
	fi

	tst_brkm TCONF "Failed to find module '$mod_name'"
}

TST_CHECKPOINT_WAIT()
{
	ROD tst_checkpoint wait 10000 "$1"
}

TST_CHECKPOINT_WAKE()
{
	ROD tst_checkpoint wake 10000 "$1" 1
}

TST_CHECKPOINT_WAKE2()
{
	ROD tst_checkpoint wake 10000 "$1" "$2"
}

TST_CHECKPOINT_WAKE_AND_WAIT()
{
	TST_CHECKPOINT_WAKE "$1"
	TST_CHECKPOINT_WAIT "$1"
}

# Check that test name is set
if [ -z "$TCID" ]; then
	tst_brkm TBROK "TCID is not defined"
fi

if [ -z "$TST_TOTAL" ]; then
	tst_brkm TBROK "TST_TOTAL is not defined"
fi

export TCID="$TCID"
export TST_TOTAL="$TST_TOTAL"

# Setup LTPROOT, default to current directory if not set
if [ -z "$LTPROOT" ]; then
	export LTPROOT="$PWD"
	export LTP_DATAROOT="$LTPROOT/datafiles"
else
	export LTP_DATAROOT="$LTPROOT/testcases/data/$TCID"
fi

if [ "$TST_NEEDS_CHECKPOINTS" = "1" ]; then
	LTP_IPC_PATH="/dev/shm/ltp_${TCID}_$$"

	LTP_IPC_SIZE=$(tst_getconf PAGESIZE)
	if [ $? -ne 0 ]; then
		tst_brkm TBROK "tst_getconf PAGESIZE failed"
	fi

	ROD_SILENT dd if=/dev/zero of="$LTP_IPC_PATH" bs="$LTP_IPC_SIZE" count=1
	ROD_SILENT chmod 600 "$LTP_IPC_PATH"
	export LTP_IPC_PATH
fi
