#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Local test, check if we can create and then delete macvlan
# interface multiple times.

virt_type="macvlan"
TST_TEST_DATA="mode private,mode vepa,mode bridge,mode passthru"
TST_TEST_DATA_IFS=","
TST_TESTFUNC=virt_test_02
. virt_lib.sh

tst_run
