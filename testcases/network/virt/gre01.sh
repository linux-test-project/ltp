#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2015-2019 Oracle and/or its affiliates.
# Author: Alexey Kodanev <alexey.kodanev@oracle.com>
#
# It requires remote host. Test will setup IPv4 and IPv6 virtual
# tunnel between two hosts, then will compare TCP performance
# with and without GRE using ping or netstress test.

TST_NEEDS_TMPDIR=1
TST_TESTFUNC=virt_netperf_msg_sizes
TST_SETUP=virt_gre_setup
TST_CLEANUP=virt_cleanup

. virt_lib.sh
tst_run
