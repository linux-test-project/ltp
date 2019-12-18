#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018-2019 Oracle and/or its affiliates. All Rights Reserved.

TST_SETUP="init"
TST_CLEANUP="cleanup"
TST_NEEDS_CMDS="iptables grep ping telnet"
TST_NEEDS_DRIVERS="ip_tables"
use_iptables=1

. iptables_lib.sh
. tst_test.sh

tst_run
