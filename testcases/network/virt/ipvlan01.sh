#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Local test, check if we can create and then delete ipvlan
# interface multiple times.

virt_type="ipvlan"
TST_TEST_DATA="mode l2,mode l3,mode l3s"
TST_TEST_DATA_IFS=","
TST_TESTFUNC=virt_test_02
. virt_lib.sh

tst_run
