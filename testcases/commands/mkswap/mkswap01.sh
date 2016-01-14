#!/bin/sh
#
# Copyright (c) 2015 Fujitsu Ltd.
# Author: Guangwen Feng <fenggw-fnst@cn.fujitsu.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# Test mkswap command with some basic options.
#

TCID=mkswap01
TST_TOTAL=10
. test.sh

setup()
{
	tst_require_root

	tst_check_cmds uuidgen blkid blockdev mkswap

	tst_tmpdir

	TST_CLEANUP=cleanup

	tst_acquire_device

	UUID=`uuidgen`

	DEVICE_SIZE=$((`blockdev --getsize64 $TST_DEVICE`/1024))

	PAGE_SIZE=`getconf PAGE_SIZE`
}

cleanup()
{
	tst_release_device

	tst_rmdir
}

wait_for_file()
{
	local path="$1"
	local retries=10

	if [ -z "$path" ]; then
		return
	fi

	while [ $retries -gt 0 ]; do
		if [ -e "$path" ]; then
			return
		fi
		tst_resm TINFO "Waiting for $path to appear"
		retries=$((retries - 1))
		tst_sleep 10ms
	done

	tst_resm TWARN "The file $path haven't appeared"
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

	wait_for_file "$dev_file"

	swapon $swapfile 2>/dev/null

	if [ $? -ne 0 ]; then
		tst_resm TINFO "Can not do swapon on $swapfile."
		if [ $pagesize -ne $PAGE_SIZE ]; then
			tst_resm TINFO "Page size specified by 'mkswap -p' \
is not equal to system's page size."
			tst_resm TINFO "Swapon failed expectedly."
			return 0
		fi

		if [ $swapsize -gt $DEVICE_SIZE ]; then
			tst_resm TINFO "Device size specified by 'mkswap' \
greater than real size."
			tst_resm TINFO "Swapon failed expectedly."
			return 0
		fi

		tst_resm TINFO "Swapon failed unexpectedly."
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
		tst_resm TWARN "Can not do swapoff on $swapfile."
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
			tst_resm TCONF "'${mkswap_cmd}' not supported."
			return
		fi

		tst_resm TFAIL "'${mkswap_cmd}' failed."
		cat temp
		return
	fi

	if [ -n "$device" ]; then
		mkswap_verify "$mkswap_op" "$op_arg" "$device" "$size" "$dev_file"
		if [ $? -ne 0 ]; then
			tst_resm TFAIL "'${mkswap_cmd}' failed, not expected."
			return
		fi
	fi

	tst_resm TPASS "'${mkswap_cmd}' passed."
}

setup

mkswap_test "" "" "$TST_DEVICE"
mkswap_test "" "" "$TST_DEVICE" "$((DEVICE_SIZE-10000))"
mkswap_test "-f" "" "$TST_DEVICE" "$((DEVICE_SIZE+10000))"
mkswap_test "-c" "" "$TST_DEVICE"
mkswap_test "-p" "2048" "$TST_DEVICE"
mkswap_test "-L" "ltp_testswap" "-L ltp_testswap" "" "/dev/disk/by-label/ltp_testswap"
mkswap_test "-v1" "" "$TST_DEVICE"
mkswap_test "-U" "$UUID" "-U $UUID" "" "/dev/disk/by-uuid/$UUID"
mkswap_test "-V"
mkswap_test "-h"

tst_exit
