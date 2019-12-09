#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015 Fujitsu Ltd.
# Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
#
# Test mkswap command with some basic options.

TST_CNT=10
TST_SETUP=setup
TST_TESTFUNC=do_test
TST_NEEDS_ROOT=1
TST_NEEDS_DEVICE=1
TST_NEEDS_CMDS="uuidgen blkid blockdev mkswap"
. tst_test.sh

setup()
{
	UUID=`uuidgen`

	PAGE_SIZE=`tst_getconf PAGESIZE`

	# Here get the size of the device and align it down to be the
	# multiple of $PAGE_SIZE and use that as the size for testing.
	real_size=`blockdev --getsize64 $TST_DEVICE`
	DEVICE_SIZE=$((($real_size/$PAGE_SIZE * $PAGE_SIZE)/1024))
}

check_for_file()
{
	local path="$1"

	if [ -z "$path" -o -e "$path" ]; then
		return
	fi
	return 1
}

mkswap_verify()
{
	local mkswap_op="$1"
	local op_arg="$2"
	local swapfile="$3"
	local dev_file="$5"

	local before=`awk '/SwapTotal/ {print $2}' /proc/meminfo`

	local swapsize=${4:-$DEVICE_SIZE}

	if [ "$mkswap_op" = "-p" ]; then
		local pagesize=$op_arg
	else
		local pagesize=$PAGE_SIZE
	fi

	if tst_kvcmp -lt "2.6.35" && [ -n "$dev_file" ]; then
		tst_res TINFO "Waiting for $dev_file to appear"
		tst_sleep 100ms
	else
		TST_RETRY_FUNC "check_for_file $dev_file" 0
	fi

	swapon $swapfile 2>/dev/null

	if [ $? -ne 0 ]; then
		tst_res TINFO "Can not do swapon on $swapfile."
		if [ $pagesize -ne $PAGE_SIZE ]; then
			tst_res TINFO "Page size specified by 'mkswap -p' \
is not equal to system's page size."
			tst_res TINFO "Swapon failed expectedly."
			return 0
		fi

		if [ $swapsize -gt $DEVICE_SIZE ]; then
			tst_res TINFO "Device size specified by 'mkswap' \
greater than real size."
			tst_res TINFO "Swapon failed expectedly."
			return 0
		fi

		tst_res TINFO "Swapon failed unexpectedly."
		return 1
	fi

	local after=`awk '/SwapTotal/ {print $2}' /proc/meminfo`

	local diff=$((after-before))
	local filesize=$((swapsize-pagesize/1024))

	local ret=0

	# In general, the swap increment by doing swapon should be equal to
	# the device size minus a page size, however for some kernels, the
	# increment we get is not exactly equal to that value, but is equal
	# to the value minus an extra page size, e.g. on RHEL5.11GA.
	if [ $diff -ne $filesize ] && \
		[ $diff -ne $((filesize-pagesize/1024)) ]; then
		ret=1
	fi

	swapoff $swapfile 2>/dev/null
	if [ $? -ne 0 ]; then
		tst_res TWARN "Can not do swapoff on $swapfile."
	fi

	return $ret
}

mkswap_test()
{
	local mkswap_op="$1"
	local op_arg="$2"
	local device="$3"
	local size="$4"
	local dev_file="$5"

	local mkswap_cmd="mkswap $mkswap_op $op_arg $TST_DEVICE $size"

	${mkswap_cmd} >temp 2>&1
	if [ $? -ne 0 ]; then
		grep -q -E "unknown option|invalid option|Usage" temp
		if [ $? -eq 0 ]; then
			tst_res TCONF "'${mkswap_cmd}' not supported."
			return
		fi

		tst_res TFAIL "'${mkswap_cmd}' failed."
		cat temp
		return
	fi

	udevadm trigger --name-match=$TST_DEVICE

	if [ -n "$device" ]; then
		mkswap_verify "$mkswap_op" "$op_arg" "$device" "$size" "$dev_file"
		if [ $? -ne 0 ]; then
			tst_res TFAIL "'${mkswap_cmd}' failed, not expected."
			return
		fi
	fi

	tst_res TPASS "'${mkswap_cmd}' passed."
}

do_test()
{
	case $1 in
	1) mkswap_test "" "" "$TST_DEVICE";;
	2) mkswap_test "" "" "$TST_DEVICE" "$((DEVICE_SIZE-PAGE_SIZE/1024))";;
	3) mkswap_test "-f" "" "$TST_DEVICE" "$((DEVICE_SIZE+PAGE_SIZE/1024))";;
	4) mkswap_test "-c" "" "$TST_DEVICE";;
	5) mkswap_test "-p" "2048" "$TST_DEVICE";;
	6) mkswap_test "-L" "ltp_testswap" "-L ltp_testswap" "" "/dev/disk/by-label/ltp_testswap";;
	7) mkswap_test "-v1" "" "$TST_DEVICE";;
	8) mkswap_test "-U" "$UUID" "-U $UUID" "" "/dev/disk/by-uuid/$UUID";;
	9) mkswap_test "-V";;
	10) mkswap_test "-h";;
	esac
}

tst_run
