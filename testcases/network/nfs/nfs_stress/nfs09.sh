#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2024 SUSE LLC <mdoucha@suse.cz>
#
# DESCRIPTION: Check that stale NFS client cache does not prevent file
# truncation.
#
# 1. Through NFS, access metadata of a file that does not exist
# 2. Create the file on the remote end (bypassing NFS)
# 3. Through NFS, try to overwrite the file with shorter data

TST_TESTFUNC="do_test"

do_test()
{
	local local_file="testfile"
	local remote_file="$(nfs_get_remote_path)/$local_file"
	local testmsg='File truncated'
	local data

	EXPECT_FAIL "ls -l '$local_file'"
	tst_rhost_run -c "echo -n 'File rewritten not' >'$remote_file'"
	echo -n "$testmsg" >"$local_file"
	data=$(tst_rhost_run -c "cat '$remote_file'")

	if [ "$data" != "$testmsg" ]; then
		tst_res TFAIL "Wrong file contents, expected '$testmsg', got '$data'"
	else
		tst_res TPASS "File was truncated correctly"
	fi
}

. nfs_lib.sh
tst_run
