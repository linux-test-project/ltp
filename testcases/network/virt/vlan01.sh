#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015 Oracle and/or its affiliates. All Rights Reserved.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# Local test, check if we can create multiple VLAN interfaces.

TCID=vlan01
TST_TOTAL=9

virt_type="vlan"

. virt_lib.sh

p0="protocol 802.1Q"
p1="protocol 802.1ad"
lb0="loose_binding off"
lb1="loose_binding on"
rh0="reorder_hdr off"
rh1="reorder_hdr on"

options=" ,$p0 $lb0 $rh0,$p0 $lb0 $rh1,$p0 $lb1 $rh0,$p0 $lb1 $rh1,\
$p1 $lb0 $rh0,$p1 $lb0 $rh1,$p1 $lb1 $rh0,$p1 $lb1 $rh1,"

virt_test_01 "$options"

tst_exit
