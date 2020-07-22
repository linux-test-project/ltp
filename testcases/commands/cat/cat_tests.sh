#!/bin/sh

TST_CNT=4
TST_TESTFUNC=do_test
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="cat"
. tst_test.sh


cat_test()
{
#create test file
	echo aaa > test

#parameter
	local cat_opt=$1
	local cat_content=$2
	local cat_cmd="cat $cat_opt $cat_content"

#command
	$cat_cmd > temp 2>&1

#logic
	if [ $? -ne 0 ]; then
		tst_res TFAIL "$cat_cmd failed."
		return
	fi

	line=`grep -R ' 1' temp | awk '{print $1}'`

	if [ -z $cat_opt ];then
		if [ -z $line ];then
			tst_res TPASS "cat passed with null option."
		else
			tst_res TFAIL "$cat_cmd failed."
		fi
		return
	else
		if [ $cat_opt = "-n" ];then
			if [ $line -ne 1 ];then
				tst_res TFAIL "$cat_cmd failed."
				return
			fi
		fi
	fi

	tst_res TPASS "cat passed with $cat_opt option."
}

do_test()
{
	case $1 in
		1) cat_test "" "test";;
		2) cat_test "-n" "test";;
		3) cat_test "--help";;
		4) cat_test "--version";;
	esac
}

tst_run
