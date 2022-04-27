#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 SUSE LLC <mdoucha@suse.cz>
#
# DESCRIPTION: Create a large number of files and directories on NFS volume.
# Then check whether they can be listed via NFS.

FILE_COUNT=5000

TST_OPTS="n:"
TST_PARSE_ARGS="do_parse_args"
TST_TESTFUNC="do_test"
TST_SETUP="do_setup"
TST_USAGE="show_usage"

do_parse_args()
{
	case "$1" in
	n) FILE_COUNT="$2";;
	esac
}

show_usage()
{
	nfs_usage
	echo "-n x    Create x files and x directories, default is 5000"
}

do_setup()
{
	nfs_setup

	local rpath=$(nfs_get_remote_path | sed -e 's/%/%%/g')
	local file_fmt="$rpath/file%1.0f"
	local dir_fmt="$rpath/dir%1.0f"

	tst_rhost_run -s -c "touch \$(seq -f \"$file_fmt\" -s ' ' $FILE_COUNT)"
	tst_rhost_run -s -c "mkdir \$(seq -f \"$dir_fmt\" -s ' ' $FILE_COUNT)"
}

do_test()
{
	local count

	# Pass the list of files through `sort -u` in case `ls` doesn't filter
	# out potential duplicate filenames returned by buggy NFS
	count=$(ls | grep '^file' | sort -u | wc -l)

	if [ $count -ne $FILE_COUNT ]; then
		tst_res TFAIL "Listing files failed: $count != $FILE_COUNT"
		return
	fi

	count=$(ls | grep '^dir' | sort -u | wc -l)

	if [ $count -ne $FILE_COUNT ]; then
		tst_res TFAIL "Listing dirs failed: $count != $FILE_COUNT"
		return
	fi

	tst_res TPASS "All files and directories were correctly listed"
}

. nfs_lib.sh
tst_run
