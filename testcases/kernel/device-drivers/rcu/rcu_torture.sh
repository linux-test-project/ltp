#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2014-2015 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (C) 2019 Xiao Yang <ice_yangxiao@163.com>
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# One of the possible ways to test RCU is to use rcutorture kernel module.
# The test requires that kernel configured with CONFIG_RCU_TORTURE_TEST.
# It runs rcutorture module using particular options and then inspects
# dmesg output for module's test results.
# For more information, please read Linux Documentation: RCU/torture.txt

TST_CNT=4
TST_SETUP=rcutorture_setup
TST_TESTFUNC=do_test
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="modprobe dmesg sed tail"
TST_OPTS="t:w:"
TST_USAGE=rcutorture_usage
TST_PARSE_ARGS=rcutorture_parse_args

# default options
test_time=30
num_writers=5

rcutorture_usage()
{
	echo "Usage:"
	echo "-t x    time in seconds for each test-case"
	echo "-w x    number of writers"
}

rcutorture_parse_args()
{
	case $1 in
	t) test_time=$2 ;;
	w) num_writers=$2 ;;
	esac
}

rcutorture_setup()
{
	local module=1

	# check if rcutorture is built as a kernel module by inserting
	# and then removing it
	modprobe -q rcutorture ||  module=
	modprobe -qr rcutorture || module=

	[ -z "$module" ] && \
		tst_brk TCONF "rcutorture is built-in, non-existent or in use"
}

rcutorture_test()
{
	local rcu_type=$1

	tst_res TINFO "$rcu_type-torture: running $test_time sec..."

	modprobe rcutorture nfakewriters=$num_writers \
		torture_type=$rcu_type >/dev/null
	if [ $? -ne 0 ]; then
		dmesg | grep -q "invalid torture type: \"$rcu_type\"" && \
			tst_brk TCONF "invalid $rcu_type type"

		tst_brk TBROK "failed to load module"
	fi

	sleep $test_time

	modprobe -r rcutorture >/dev/null || \
		tst_brk TBROK "failed to unload module"

	# check module status in dmesg
	local res=$(dmesg | sed -nE "s/.* $rcu_type-torture:.* End of test: (.*): .*/\1/p" | tail -n1)
	if [ "$res" = "SUCCESS" ]; then
		tst_res TPASS "$rcu_type-torture: $res"
	else
		tst_res TFAIL "$rcu_type-torture: $res, see dmesg"
	fi
}

do_test()
{
	case $1 in
	1) rcutorture_test rcu;;
	2) rcutorture_test srcu;;
	3) rcutorture_test srcud;;
	4) rcutorture_test tasks;;
	esac
}

. tst_test.sh
tst_run
