#!/bin/sh
#
# Copyright (c) Linux Test Project, 2014
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
export TST_LIB_LOADED=1

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
	tst_flag2mask "$1"
	local mask=$?
	LTP_RET_VAL=$((LTP_RET_VAL|mask))

	local ret=$1
	shift
	echo "$TCID $TST_COUNT $ret : $@"

	case "$ret" in
	TPASS|TFAIL)
	TST_COUNT=$((TST_COUNT+1));;
	esac
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
	if [ -n "$TST_CLEANUP" ]; then
		$TST_CLEANUP
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
	cd "$TST_TMPDIR"
}

tst_rmdir()
{
	cd "$LTPROOT"
	rm -r "$TST_TMPDIR"
}

#
# Checks if commands passed as arguments exists
#
tst_check_cmds()
{
	for cmd in $*; do
		if ! command -v $cmd > /dev/null 2>&1; then
			tst_brkm TCONF "'$cmd' not found"
		fi
	done
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
	$@ > /dev/null 2>&1
	if [ $? -ne 0 ]; then
		tst_brkm TBROK "$@ failed"
	fi
}

ROD()
{
	$@
	if [ $? -ne 0 ]; then
		tst_brkm TBROK "$@ failed"
	fi
}

tst_acquire_device()
{
	if [ -z ${TST_TMPDIR} ]; then
		tst_brkm "Use 'tst_tmpdir' before 'tst_acquire_device'"
	fi

	if [ -n "${LTP_DEV}" ]; then
		tst_resm TINFO "Using test device LTP_DEV='${LTP_DEV}'"
		if [ ! -b ${LTP_DEV} ]; then
			tst_brkm TBROK "${LTP_DEV} is not a block device"
		fi
		TST_DEVICE=${LTP_DEV}
		TST_DEVICE_FLAG=0
		return
	fi

	ROD_SILENT dd if=/dev/zero of=test_dev.img bs=1024 count=20480

	TST_DEVICE=$(losetup -f)
	if [ $? -ne 0 ]; then
		tst_brkm TBROK "Couldn't find free loop device"
	fi

	tst_resm TINFO "Found free device '${TST_DEVICE}'"

	ROD_SILENT losetup ${TST_DEVICE} test_dev.img

	TST_DEVICE_FLAG=1
}

tst_release_device()
{
	if [ ${TST_DEVICE_FLAG} -eq 0 ]; then
		return
	fi

	losetup -a | grep -q ${TST_DEVICE}
	if [ $? -eq 0 ]; then
		losetup -d ${TST_DEVICE}
		if [ $? -ne 0 ];then
			tst_resm TWARN "'losetup -d ${TST_DEVICE}' failed"
		fi
	fi
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
