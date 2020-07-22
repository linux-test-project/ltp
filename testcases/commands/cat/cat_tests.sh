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
#创建临时测试文件
    touch test
    echo aaa > test
#定义2个局部变量，并赋值$1和$2
    local cat_opt=$1
	local cat_content=$2

#定义1个局部变量，拼接要测试的命令
	local cat_cmd="cat $cat_opt $cat_content"

#把命令执行的结果放入temp文件，>表示覆盖写入，>>表示追加写入，2>&1表示执行错误和正常的信息都写入这个文件。
    $cat_cmd > temp 2>&1
#下面进入逻辑，$?表示命令执行的结果，-ne表示不等于，不等于0，肯定就是报错啦，我们就用tst_res抛出一个错误日志方便定位，至于TCONF，TFAIL是ltp的日志级别，很眼熟吧。
  if [ $? -ne 0 ]; then
    tst_res TFAIL "$cat_cmd failed."
    return
  fi

#cat -n会把内容写上行号，我们用‘ 1’来通过grep命令过滤执行的内容并拿到行号1，下面会通过判断行号1是否存在来判断cat -n是否执行成功。
  line=`grep -R ' 1' temp | awk '{print $1}'`

#下面的判断-z表示“STRING” 的长度为零则为真，do_test()里面第一个测试项中cat_opt是空字符串，满足了，继续判断-ne表示不等于1，这里执行的是没有带-n参数，因此不会有编号，直接执行成功 return，当cat_opt为-n时，会有编号1，不等于1表示执行失败。
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

#经过以上重重的错误判断筛选，通关后终于得到下面的执行成功的日志。
  tst_res TPASS "cat passed with $cat_opt option."
#删除临时测试文件
rm test
}

#入口在这里，我们可以看到里面调用的是一个case循环，每次调用cat_test方法进行执行，注意cat_test后面的第一个和第二个双引号的参数就是$1和$2的输入值，好了，我们回到cat_test方法
do_test()
{
	case $1 in
		1) cat_test "" "test";;
		2) cat_test "-n" "test";;
		3) cat_test "--help";;
		4) cat_test "--version";;
	esac
}

#LTP框架的内容
tst_run
