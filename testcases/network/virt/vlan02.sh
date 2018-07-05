#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Local test, check if we can create and then delete VLAN
# interface 4095 times.

virt_type="vlan"

TST_TESTFUNC=do_test
. virt_lib.sh

do_test()
{
	virt_add_delete_test "id 4094"
}

tst_run
