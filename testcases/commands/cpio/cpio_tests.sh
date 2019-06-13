#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Test basic functionality of cpio command

TST_TESTFUNC=cpio_test
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="cpio"
. tst_test.sh

cpio_test()
{
	ROD mkdir "dir"
	for i in a b c d e f g h i j k l m n o p q r s t u v w x y z; do
		ROD echo "Test" > "dir/$i"
	done

	ROD find dir -type f > filelist
	EXPECT_PASS cpio -o \> cpio.out \< filelist
	ROD mv "dir" "dir_orig"
	ROD mkdir "dir"
	EXPECT_PASS cpio -i \< cpio.out

	if diff -r "dir" "dir_orig"; then
		tst_res TPASS "Directories dir and dir_orig are equal"
	else
		tst_res TFAIL "Directories dir and dir_orig differ"
		ls -R dir_orig
		echo
		ls -R dir
	fi
}

tst_run
