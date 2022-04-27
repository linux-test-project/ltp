#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2014-2015 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Local test, check if we can create and then delete VXLAN
# interface 5000 times.

TST_OPTS="hi:d:"
TST_PARSE_ARGS=virt_lib_parse_args
TST_USAGE=virt_lib_usage
TST_TESTFUNC=do_test

virt_type="vxlan"
start_id=16700000

do_test()
{
	local mult_addr="239.1.1.1"
	[ "$TST_IPV6" ] && mult_addr="ff02::abc"

	virt_add_delete_test "id $start_id group $mult_addr"
}

. virt_lib.sh
tst_run
