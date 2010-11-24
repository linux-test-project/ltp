#!/bin/bash
#
# Run mce test cases in a loop. It exists on failure of any one of the test cases.
# This script is using simple test driver.
#
#   Authors: Dean Nelson <dnelson@redhat.com>
#           Zheng Jiajia <jiajia.zheng@intel.com>
# This file is released under the GPLv2.
#
# Usage:
#Run as root and invoke this test tool on test configure file. 
#For example, ./loop-mce-test simple_ser.conf
#Note that only simple test configure file is used and full path is not needed here.

sd=$(dirname "$0")
export ROOT=`(cd $sd/..; pwd)`
prog=$(basename "$0")

usage()
{
    echo "$prog <config>"
    exit 1
}

. $ROOT/lib/functions.sh

[ $# -eq 1 ] || usage
config=$(basename "$1")
[ -f $ROOT/config/$config ] || usage

i=0
while true ; do
	((i=i+1))
	echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!! loop $i"
	rm $ROOT/results/simple/result

	sh $ROOT/drivers/simple/driver.sh $ROOT/config/$1

	sed -e'/gcov/d' $ROOT/results/simple/result | grep "Fail" > /dev/null
	if [ $? = 0 ] ; then
		echo "failed on loop $i"
		exit 1
	fi
done

