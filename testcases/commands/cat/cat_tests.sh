#!/bin/sh

#下面的语句表示会执行4个测试项
TST_CNT=4
#下面的语句表示从do_test方法开始执行
TST_TESTFUNC=do_test
#下面的语句表示需要一个临时目录
TST_NEEDS_TMPDIR=1
#下面的语句表示需要执行的命令为cat
TST_NEEDS_CMDS="cat"
#下面的语句是LTP的规范
. tst_test.sh


#下面有2个方法，cat_test()和do_test()，下面我们进入执行的入口do_test方法，然后再回来这里，这下我们可以用local定义局部变量，避免影响其他用例，现在我们可以知道下面的$1和$2的值了，拿到他们往下走。
cat_test()
{
#create test file
	touch test
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

#delete test file
	rm test
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
