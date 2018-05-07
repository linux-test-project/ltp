#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Local test, check if we can create and then delete VLAN
# interface 4095 times.

TCID=vlan02
TST_TOTAL=1

virt_type="vlan"

. virt_lib.sh

virt_add_delete_test "id 4094"

tst_exit
