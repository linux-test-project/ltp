#! /bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2015-2018 Oracle and/or its affiliates. All Rights Reserved.
# Copyright (c) International Business Machines  Corp., 2001
# Created by: Robbie Williamson (robbiew@us.ibm.com)

TST_CLEANUP="nfs03_cleanup"
TST_SETUP="nfs03_setup"
TST_TESTFUNC="do_test"

DIR_NUM=${DIR_NUM:-"100"}
FILE_NUM=${FILE_NUM:-"100"}
THREAD_NUM=${THREAD_NUM:-"1"}
ORIG_NFSD=

make_subdirs()
{
	tst_res TINFO "make '$DIR_NUM' directories"
	for i in $(seq 0 $DIR_NUM); do
		ROD mkdir -p dir$i
	done
}

touch_files()
{
	for j in $(seq 0 $DIR_NUM); do
		cd dir$j
		for k in $(seq 0 $FILE_NUM); do
			ROD touch file$j$k
		done
		cd ..
	done
}

rm_files()
{
	for j in $(seq 0 $DIR_NUM); do
		cd dir$j
		for k in $(seq 0 $FILE_NUM); do
			ROD rm -f file$j$k
		done
		cd ..
	done
}

do_test()
{
	tst_res TINFO "Multiple processes creating and deleting files"

	tst_res TINFO "creating dir1 subdirectories & files"
	ROD mkdir -p dir1
	cd dir1
	make_subdirs
	touch_files &
	pid1=$!
	cd ..

	tst_res TINFO "creating dir2 subdirectories & files"
	ROD mkdir -p dir2
	cd dir2
	make_subdirs
	touch_files &
	pid2=$!

	tst_res TINFO "cd dir1 & removing files"
	cd ../dir1
	wait $pid1
	rm_files &
	pid3=$!

	tst_res TINFO "cd dir2 & removing files"
	cd ../dir2
	wait $pid2
	rm_files
	wait $pid3

	tst_res TPASS "test done"
}

nfs03_setup()
{
	nfs_setup

	tst_res TINFO "Setting server side nfsd count to $THREAD_NUM"
	ORIG_NFSD=$(tst_rhost_run -s -c 'ps -ef | grep -w nfsd | grep -v grep | wc -l')
	tst_rhost_run -s -c "rpc.nfsd $THREAD_NUM"
}

nfs03_cleanup()
{
	tst_rhost_run -c "rpc.nfsd $ORIG_NFSD"
	nfs_cleanup
}

. nfs_lib.sh
tst_run
