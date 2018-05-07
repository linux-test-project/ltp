#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Local test, check if we can create and then delete ipvlan
# interface multiple times.

TCID=ipvlan01
TST_TOTAL=2

virt_type="ipvlan"

. virt_lib.sh

options="mode l2,mode l3"

virt_test_02 "$options"

tst_exit
