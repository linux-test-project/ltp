#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2024 SUSE LLC <mdoucha@suse.cz>

TST_TESTFUNC="do_test"

# PURPOSE:  Check that /proc/net/rpc/nfs exists in nested network namespaces
# d47151b79e32 ("nfs: expose /proc/net/sunrpc/nfs in net namespaces")
# part of patchset
# https://lore.kernel.org/linux-nfs/cover.1708026931.git.josef@toxicpanda.com/
do_test()
{
	local procfile="/proc/net/rpc/nfs"

	if tst_rhost_run -c "test -e '$procfile'"; then
		tst_res TPASS "$procfile exists in net namespaces"
	else
		tst_res TFAIL "$procfile missing in net namespaces"
	fi
}

# Force use of nested net namespace
unset RHOST

. nfs_lib.sh
tst_run
