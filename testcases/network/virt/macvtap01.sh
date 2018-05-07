#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Local test, check if we can create and then delete macvtap
# interface multiple times.

TCID=macvtap01
TST_TOTAL=4

virt_type="macvtap"

. virt_lib.sh

options="mode private,mode vepa,mode bridge,mode passthru"

virt_test_02 "$options"

tst_exit
