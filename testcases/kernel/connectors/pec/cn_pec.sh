#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2008 FUJITSU LIMITED
# Copyright (c) 2021 Joerg Vehlow <joerg.vehlow@aox-tech.de>
# Copyright (c) 2021 Petr Vorel <pvorel@suse.cz>
#
# Author: Li Zefan <lizf@cn.fujitsu.com>
#
# Process event connector is a netlink connector that reports process events
# to userspace. It sends events such as fork, exec, id change and exit.

TST_SETUP=setup
TST_TESTFUNC=test
TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_TEST_DATA="fork exec exit uid gid"

NUM_EVENTS=1

. tst_test.sh

setup()
{
	if ! grep -q cn_proc /proc/net/connector; then
		tst_brk TCONF "Process Event Connector is not supported or kernel < 2.6.26"
	fi

	tst_res TINFO "Test process events connector"
}

test()
{
	local event=$2
	local expected_events lis_rc pid

	pec_listener >lis_$event.log 2>lis_$event.err &
	pid=$!
	# wait for pec_listener to start listening
	tst_sleep 100ms

	# generator must be in PATH
	ROD event_generator -n $NUM_EVENTS -e $event \>gen_$event.log 2\>gen_$event.err

	# sleep until pec_listener has seen and handled all of the generated events
	tst_sleep 100ms
	kill -s INT $pid 2> /dev/null
	wait $pid
	lis_rc=$?

	if [ ! -s gen_$event.log ]; then
		tst_brk TBROK "failed to generate process events"
	fi

	if [ $lis_rc -ne 0 ]; then
		tst_brk TBROK "failed to execute the listener: $(cat lis_$event.err)"
	fi

	expected_events="$(cat gen_$event.log)"
	if grep -q "$expected_events" lis_$event.log; then
		tst_res TPASS "$event detected by listener"
	else
		tst_res TFAIL "$event not detected by listener"
	fi
}

tst_run
