#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

TST_NEEDS_ROOT=1
TST_SETUP="setup"
TST_CLEANUP="route_cleanup"
TST_NEEDS_CMDS="ip"
TST_CNT=$NS_TIMES

. tst_net.sh

route_cleanup()
{
	tst_restore_ipaddr
	tst_restore_ipaddr rhost
}
