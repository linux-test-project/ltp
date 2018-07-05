#!/bin/sh
#
# Copyright (c) Linux Test Project, 2014-2018
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

[ -n "$TST_LIB_LOADED" ] && return 0

export TST_PASS=0
export TST_FAIL=0
export TST_BROK=0
export TST_WARN=0
export TST_CONF=0
export TST_COUNT=1
export TST_ITERATIONS=1
export TST_TMPDIR_RHOST=0
export TST_LIB_LOADED=1

. tst_ansi_color.sh

_tst_do_exit()
{
	local ret=0

	if [ -n "$TST_SETUP_STARTED" -a -n "$TST_CLEANUP" -a \
	     -z "$TST_NO_CLEANUP" ]; then
		$TST_CLEANUP
	fi

	if [ "$TST_NEEDS_DEVICE" = 1 -a "$TST_DEVICE_FLAG" = 1 ]; then
		if ! tst_device release "$TST_DEVICE"; then
			tst_res TWARN "Failed to release device '$TST_DEVICE'"
		fi
	fi

	if [ "$TST_NEEDS_TMPDIR" = 1 -a -n "$TST_TMPDIR" ]; then
		cd "$LTPROOT"
		rm -r "$TST_TMPDIR"
		[ "$TST_TMPDIR_RHOST" = 1 ] && tst_cleanup_rhost
	fi

	if [ $TST_FAIL -gt 0 ]; then
		ret=$((ret|1))
	fi

	if [ $TST_BROK -gt 0 ]; then
		ret=$((ret|2))
	fi

	if [ $TST_WARN -gt 0 ]; then
		ret=$((ret|4))
	fi

	if [ $TST_CONF -gt 0 ]; then
		ret=$((ret|32))
	fi

	echo
	echo "Summary:"
	echo "passed   $TST_PASS"
	echo "failed   $TST_FAIL"
	echo "skipped  $TST_CONF"
	echo "warnings $TST_WARN"

	exit $ret
}

_tst_inc_res()
{
	case "$1" in
	TPASS) TST_PASS=$((TST_PASS+1));;
	TFAIL) TST_FAIL=$((TST_FAIL+1));;
	TBROK) TST_BROK=$((TST_BROK+1));;
	TWARN) TST_WARN=$((TST_WARN+1));;
	TCONF) TST_CONF=$((TST_CONF+1));;
	TINFO) ;;
	*) tst_brk TBROK "Invalid res type '$1'";;
	esac
}

tst_res()
{
	local res=$1
	shift

	tst_color_enabled
	local color=$?

	_tst_inc_res "$res"

	printf "$TST_ID $TST_COUNT "
	tst_print_colored $res "$res: "
	echo "$@"
}

tst_brk()
{
	local res=$1
	shift

	tst_res "$res" "$@"
	_tst_do_exit
}

ROD_SILENT()
{
	local tst_out="$(tst_rod $@ 2>&1)"
	if [ $? -ne 0 ]; then
		echo "$tst_out"
		tst_brk TBROK "$@ failed"
	fi
}

ROD()
{
	tst_rod "$@"
	if [ $? -ne 0 ]; then
		tst_brk TBROK "$@ failed"
	fi
}

EXPECT_PASS()
{
	tst_rod "$@"
	if [ $? -eq 0 ]; then
		tst_res TPASS "$@ passed as expected"
	else
		tst_res TFAIL "$@ failed unexpectedly"
	fi
}

EXPECT_FAIL()
{
	# redirect stderr since we expect the command to fail
	tst_rod "$@" 2> /dev/null
	if [ $? -ne 0 ]; then
		tst_res TPASS "$@ failed as expected"
	else
		tst_res TFAIL "$@ passed unexpectedly"
	fi
}

TST_RETRY_FN_EXP_BACKOFF()
{
	local tst_fun="$1"
	local tst_exp=$2
	local tst_sec=$(expr $3 \* 1000000)
	local tst_delay=1

	if [ $# -ne 3 ]; then
		tst_brk TBROK "TST_RETRY_FN_EXP_BACKOFF expects 3 parameters"
	fi

	if ! tst_is_int "$tst_sec"; then
		tst_brk TBROK "TST_RETRY_FN_EXP_BACKOFF: tst_sec must be integer ('$tst_sec')"
	fi

	while true; do
		$tst_fun
		if [ "$?" = "$tst_exp" ]; then
			break
		fi

		if [ $tst_delay -lt $tst_sec ]; then
			tst_sleep ${tst_delay}us
			tst_delay=$((tst_delay*2))
		else
			tst_brk TBROK "\"$tst_fun\" timed out"
		fi
	done

	return $tst_exp
}

TST_RETRY_FUNC()
{
	if [ $# -ne 2 ]; then
		tst_brk TBROK "TST_RETRY_FUNC expects 2 parameters"
	fi

	TST_RETRY_FN_EXP_BACKOFF "$1" "$2" 1
	return $2
}

tst_umount()
{
	local device="$1"
	local i=0

	if ! grep -q "$device" /proc/mounts; then
		tst_res TINFO "The $device is not mounted, skipping umount"
		return
	fi

	while [ "$i" -lt 50 ]; do
		if umount "$device" > /dev/null; then
			return
		fi

		i=$((i+1))

		tst_res TINFO "umount($device) failed, try $i ..."
		tst_res TINFO "Likely gvfsd-trash is probing newly mounted "\
		              "fs, kill it to speed up tests."

		tst_sleep 100ms
	done

	tst_res TWARN "Failed to umount($device) after 50 retries"
}

tst_mkfs()
{
	local fs_type=$1
	local device=$2
	shift 2
	local fs_opts="$@"

	if [ -z "$fs_type" ]; then
		tst_brk TBROK "No fs_type specified"
	fi

	if [ -z "$device" ]; then
		tst_brk TBROK "No device specified"
	fi

	tst_res TINFO "Formatting $device with $fs_type extra opts='$fs_opts'"

	ROD_SILENT mkfs.$fs_type $fs_opts $device
}

tst_check_cmds()
{
	local cmd
	for cmd in $*; do
		if ! command -v $cmd > /dev/null 2>&1; then
			tst_brk TCONF "'$cmd' not found"
		fi
	done
}

tst_is_int()
{
	[ "$1" -eq "$1" ] 2>/dev/null
	return $?
}

tst_usage()
{
	if [ -n "$TST_USAGE" ]; then
		$TST_USAGE
	else
		echo "usage: $0"
		echo "OPTIONS"
	fi

	echo "-h      Prints this help"
	echo "-i n    Execute test n times"
}

_tst_resstr()
{
	echo "$TST_PASS$TST_FAIL$TST_CONF"
}

_tst_rescmp()
{
	local res=$(_tst_resstr)

	if [ "$1" = "$res" ]; then
		tst_brk TBROK "Test didn't report any results"
	fi
}

tst_run()
{
	local _tst_i
	local _tst_data
	local _tst_max
	local _tst_name

	if [ -n "$TST_TEST_PATH" ]; then
		for _tst_i in $(grep TST_ "$TST_TEST_PATH" | sed 's/.*TST_//; s/[="} \t\/:`].*//'); do
			case "$_tst_i" in
			SETUP|CLEANUP|TESTFUNC|ID|CNT|MIN_KVER);;
			OPTS|USAGE|PARSE_ARGS|POS_ARGS);;
			NEEDS_ROOT|NEEDS_TMPDIR|NEEDS_DEVICE|DEVICE);;
			NEEDS_CMDS|NEEDS_MODULE|MODPATH|DATAROOT);;
			IPV6|IPVER|TEST_DATA|TEST_DATA_IFS);;
			RETRY_FUNC|RETRY_FN_EXP_BACKOFF);;
			*) tst_res TWARN "Reserved variable TST_$_tst_i used!";;
			esac
		done

		for _tst_i in $(grep _tst_ "$TST_TEST_PATH" | sed 's/.*_tst_//; s/[="} \t\/:`].*//'); do
			tst_res TWARN "Private variable or function _tst_$_tst_i used!"
		done
	fi

	OPTIND=1

	while getopts "hi:$TST_OPTS" _tst_name $TST_ARGS; do
		case $_tst_name in
		'h') tst_usage; exit 0;;
		'i') TST_ITERATIONS=$OPTARG;;
		'?') tst_usage; exit 2;;
		*) $TST_PARSE_ARGS "$_tst_name" "$OPTARG";;
		esac
	done

	if ! tst_is_int "$TST_ITERATIONS"; then
		tst_brk TBROK "Expected number (-i) not '$TST_ITERATIONS'"
	fi

	if [ "$TST_ITERATIONS" -le 0 ]; then
		tst_brk TBROK "Number of iterations (-i) must be > 0"
	fi

	if [ "$TST_NEEDS_ROOT" = 1 ]; then
		if [ "$(id -ru)" != 0 ]; then
			tst_brk TCONF "Must be super/root for this test!"
		fi
	fi

	tst_check_cmds $TST_NEEDS_CMDS

	if [ -n "$TST_MIN_KVER" ]; then
		tst_kvcmp -lt "$TST_MIN_KVER" && \
			tst_brk TCONF "test requires kernel $TST_MIN_KVER+"
	fi

	if [ "$TST_NEEDS_TMPDIR" = 1 ]; then
		if [ -z "$TMPDIR" ]; then
			export TMPDIR="/tmp"
		fi

		TST_TMPDIR=$(mktemp -d "$TMPDIR/LTP_$TST_ID.XXXXXXXXXX")

		chmod 777 "$TST_TMPDIR"

		TST_STARTWD=$(pwd)

		cd "$TST_TMPDIR"
	fi

	if [ "$TST_NEEDS_DEVICE" = 1 ]; then
		if [ -z ${TST_TMPDIR} ]; then
			tst_brk TBROK "Use TST_NEEDS_TMPDIR must be set for TST_NEEDS_DEVICE"
		fi

		TST_DEVICE=$(tst_device acquire)

		if [ ! -b "$TST_DEVICE" -o $? -ne 0 ]; then
			tst_brk TBROK "Failed to acquire device"
		fi

		TST_DEVICE_FLAG=1
	fi

	if [ -n "$TST_NEEDS_MODULE" ]; then
		for tst_module in "$TST_NEEDS_MODULE" \
		                  "$LTPROOT/testcases/bin/$TST_NEEDS_MODULE" \
		                  "$TST_STARTWD/$TST_NEEDS_MODULE"; do

				if [ -f "$tst_module" ]; then
					TST_MODPATH="$tst_module"
					break
				fi
		done

		if [ -z "$TST_MODPATH" ]; then
			tst_brk TCONF "Failed to find module '$TST_NEEDS_MODULE'"
		else
			tst_res TINFO "Found module at '$TST_MODPATH'"
		fi
	fi

	if [ -n "$TST_SETUP" ]; then
		TST_SETUP_STARTED=1
		$TST_SETUP
	fi

	#TODO check that test reports some results for each test function call
	while [ $TST_ITERATIONS -gt 0 ]; do
		if [ -n "$TST_TEST_DATA" ]; then
			tst_check_cmds cut tr wc
			_tst_max=$(( $(echo $TST_TEST_DATA | tr -cd "$TST_TEST_DATA_IFS" | wc -c) +1))
			for _tst_i in $(seq $_tst_max); do
				_tst_data="$(echo "$TST_TEST_DATA" | cut -d"$TST_TEST_DATA_IFS" -f$_tst_i)"
				_tst_run_tests "$_tst_data"
			done
		else
			_tst_run_tests
		fi
		TST_ITERATIONS=$((TST_ITERATIONS-1))
	done

	_tst_do_exit
}

_tst_run_tests()
{
	local _tst_data="$1"
	local _tst_i

	for _tst_i in $(seq ${TST_CNT:-1}); do
		if type ${TST_TESTFUNC}1 > /dev/null 2>&1; then
			_tst_run_test "$TST_TESTFUNC$_tst_i" $_tst_i "$_tst_data"
		else
			_tst_run_test "$TST_TESTFUNC" $_tst_i "$_tst_data"
		fi
	done
}

_tst_run_test()
{
	local _tst_res=$(_tst_resstr)
	local _tst_fnc="$1"
	shift

	$_tst_fnc "$@"
	_tst_rescmp "$_tst_res"
	TST_COUNT=$((TST_COUNT+1))
}

if [ -z "$TST_ID" ]; then
	_tst_filename=$(basename $0) || \
		tst_brk TCONF "Failed to set TST_ID from \$0 ('$0'), fix it with setting TST_ID before sourcing tst_test.sh"
	TST_ID=${_tst_filename%%.*}
fi
export TST_ID="$TST_ID"

if [ -z "$LTPROOT" ]; then
	export LTPROOT="$PWD"
	export TST_DATAROOT="$LTPROOT/datafiles"
else
	export TST_DATAROOT="$LTPROOT/testcases/data/$TST_ID"
fi

if [ -z "$TST_NO_DEFAULT_RUN" ]; then
	if TST_TEST_PATH=$(which $0) 2>/dev/null; then
		if ! grep -q tst_run "$TST_TEST_PATH"; then
			tst_brk TBROK "Test $0 must call tst_run!"
		fi
	fi

	if [ -z "$TST_TESTFUNC" ]; then
		tst_brk TBROK "TST_TESTFUNC is not defined"
	fi

	TST_TEST_DATA_IFS="${TST_TEST_DATA_IFS:- }"

	if [ -n "$TST_CNT" ]; then
		if ! tst_is_int "$TST_CNT"; then
			tst_brk TBROK "TST_CNT must be integer"
		fi

		if [ "$TST_CNT" -le 0 ]; then
			tst_brk TBROK "TST_CNT must be > 0"
		fi
	fi

	if [ -n "$TST_POS_ARGS" ]; then
		if ! tst_is_int "$TST_POS_ARGS"; then
			tst_brk TBROK "TST_POS_ARGS must be integer"
		fi

		if [ "$TST_POS_ARGS" -le 0 ]; then
			tst_brk TBROK "TST_POS_ARGS must be > 0"
		fi
	fi

	TST_ARGS="$@"

	while getopts ":hi:$TST_OPTS" tst_name; do
		case $tst_name in
		'h') TST_PRINT_HELP=1;;
		*);;
		esac
	done

	shift $((OPTIND - 1))

	if [ -n "$TST_POS_ARGS" ]; then
		if [ -z "$TST_PRINT_HELP" -a $# -ne "$TST_POS_ARGS" ]; then
			tst_brk TBROK "Invalid number of positional parameters:"\
					  "have ($@) $#, expected ${TST_POS_ARGS}"
		fi
	else
		if [ -z "$TST_PRINT_HELP" -a $# -ne 0 ]; then
			tst_brk TBROK "Unexpected positional arguments '$@'"
		fi
	fi
fi
