#/bin/bash
#
# File system metadata stress testing script v0.1 
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; version
# 2.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should find a copy of v2 of the GNU General Public License somewhere
# on your Linux system; if not, write to the Free Software Foundation, 
# Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA 
#
# Copyright (C) 2009, Intel Corp.
# Author: Shaohui Zheng <shaohui.zheng@intel.com>

export K_CWD=$(cd $(dirname $0)>/dev/null;pwd)
export K_VAR=$K_CWD/var
export K_TREE_GEN=$K_CWD/k-tree-gen
export K_TREE_TRAV=$K_CWD/k-tree-trav



function summary_result()
{
	local total_nr=$(egrep "pass|fail" $K_RESULT | wc -l ) 
	local pass_nr=$(grep pass $K_RESULT| wc -l ) 
	local fail_nr=$(grep fail $K_RESULT| wc -l ) 
	local end_ts=$(date +%s)
	local run_time=$(expr $end_ts - $K_START_TS)

	k_result "Finish fs-metadata testing within $run_time secs, $total_nr metadata "
	k_result "testing finished."
	k_result "PASS:$pass_nr"
	k_result "FAIL:$fail_nr"
	k_result "You can refer to result file $K_RESULT,"
	k_result "and log file $K_LOG for details."

	# cleanup
	for pid in $(cat $K_THREADS_PID)
	do
		k_log "killing k-thread $pid"
		run_quiet kill -9 $pid
	done
	: > $K_THREADS_PID
}

# run program and do not display the output
function run_quiet()
{
	local cmd=$*
	$cmd >/dev/null 2>&1
	return $?
}

function abort_test()
{
	k_result "finish $sec secs fs stress testing, clean up the envirnment"
	
	# summary the test result
	run_quiet unlink $K_FLAG
	summary_result
	exit 0
}

function usage()
{
	alert "File system metadata testing script v0.12 \n"
	echo ""
	echo "This script creates a lot of directory entries with k-tree data structure, "
	echo "It covers i-node creation/removing/linking/unliking operation in heavy I/O "
	echo "workloads."
	echo ""
	echo "A k-tree is a tranformation of binary tree, A binary has 2 children at most"
	echo "in each node, but a k-tree has k sub-nodes. We test both file and directory"
	echo "entries, So we do some changes. We create k sub directories and k text file"
	echo "in each parent."
	echo ""
	echo "We will caculate approximate disk space, it depends on your parameter. For  "
	echo "tree_depth, suggest to less than 10. For node_number, suggest to less 20.  "
	echo "If you pass a large number as parameter, it generate a huge directory entry"
	echo "it exhaust your disk space very fast, very hard to remove."
	alert "\nWe suggest you to run the script in a standalone partition, you can format\n"
	alert "it after test finished!\n"
	echo ""
	echo "Usage: "
	echo -e "\ttree_depth node_number threads run_time(secs) [result_file] [temp_dir] [log_file]\n"
	exit 0
}

function alert()
{
	echo -en "\\033[40;31m" # set font color as red
	echo -en "$*"
	echo -en "\\033[0;39m" # restore font color as normal
}

function k_result()
{
	echo [$(date "+%m-%d %H:%M:%S")] $* | tee -a $K_LOG
	echo $* | egrep "pass|fail" 
	ret_val=$?
	if [ -f $K_FLAG ] ;then
		 echo [$(date "+%m-%d %H:%M:%S")] $* >> $K_RESULT
	fi

	if [ ! -f $K_FLAG ] && [ $ret_val -ne 0 ] ;then
		 echo [$(date "+%m-%d %H:%M:%S")] $* >> $K_RESULT
	fi
}

function check_disk_space()
{
	run_quiet cd $K_CWD 
	local depth=$(expr $1 + 1)
	local width=$2
	local thread=$3
	local tree_size=$(echo "scale=2; (1-$width^$depth)/(1-$width) *2*4/1024 " | bc)
	local total_size=$(echo "scale=2; $tree_size * 2 * $thread " | bc)
	local free_space=$( df . -m  | awk '{ print $3}' | tail -1)

	k_log "The k-tree size is $tree_size M, the total free space reqirements is about $total_size M."

	local ready=$(echo $free_space $total_size | awk '{if($1>$2){printf "0"}else{printf"1"}}')
	if [ $ready -ne 0 ];then
		 k_log "You have $free_space M free space only, we can not finish your testing, abort it."
		 return 1
	else
		 k_log "You have $free_space M free space, ready to run the testing."
		 run_quiet cd -
		 return 0
	fi
}

function k_log()
{
	echo [$(date "+%m-%d %H:%M:%S")] $* | tee -a $K_LOG 
}

function k_thread()
{
	local dir=$1 
	local depth=$2
	local width=$3 
	local result

	# generate new tree
	k_log "begin to generate tree $dir"
	$K_TREE_GEN $depth $width
	k_log "end to generate tree $dir"

	cwd=$(pwd)
	run_quiet cd $K_VAR
	while [ -e $K_FLAG ]
	do
		new_dir=$dir-new
		cp $dir $new_dir -pr
		run_quiet cd $new_dir
		k_log "thread $dir: begin to traverse dir $new_dir"
		$K_TREE_TRAV $2 $3
		k_log "thread $dir: end to traverse dir $new_dir"
		run_quiet cd -
		result=PASS
		k_tree_diff $dir $new_dir || result=FAIL
		k_result "thread $dir: $result to compare result between dir $dir and $new_dir" 
		
		rm $new_dir -fr
	done
	run_quiet cd $cwd 
	# test ends, remove the oringal tree
	rm $dir -fr
}



if [ $# -lt 4 ] ;then
	usage
	k_result "[end] invalid input $*"
fi

if [ $# -gt 4 ];then
	K_RESULT=$5
	if [ $(dirname $K_RESULT) = '.' ]; then
		 K_RESULT=$K_CWD/$K_RESULT 
	fi
else
	K_RESULT=$K_CWD/result.txt
fi

if [ $# -gt 5 ];then
	K_VAR=$6
	if [ $(dirname $K_VAR) = '.' ]; then
		 K_VAR=$K_CWD/$K_VAR 
	fi
fi

if [ $# -gt 6 ];then
	K_LOG=$7
	if [ $(dirname $K_RESULT) = '.' ]; then
		 K_LOG=$K_VAR/$K_LOG 
	fi
else
	K_LOG=$K_VAR/log.txt
fi
export K_FLAG=$K_VAR/fs_flag
export K_START_TS=
export K_THREADS_PID=$K_VAR/k-threads.pid
export K_FS_METADATA_PID=$K_VAR/fs_metadata.pid
export K_RESULT
export K_LOG
: > $K_RESULT
: > $K_LOG

#main portal
[ -e $K_FLAG ] && unlink $K_FLAG
k_result "[begin] fs-stress testing start with parameters: $*"

check_disk_space $*
if [ $? -ne 0 ];then
	exit 1
fi

#clean up
k_log "clean up testing environment"
[ -d $K_VAR ]  && rm $K_VAR -fr
mkdir -p $K_VAR 

touch $K_FS_METADATA_PID
touch $K_THREADS_PID
echo $$ > $K_FS_METADATA_PID
: > $K_THREADS_PID
touch $K_FLAG

run_quiet cd $K_VAR

K_START_TS=$(date +%s)

# signal handler
trap abort_test 0

#testing
counter=0
while [ $counter -lt $3 ];
do
	mkdir $counter -p
	cur=$(pwd)
	run_quiet cd $counter
	#k_thread $counter $1 $2 &
	$K_CWD/k-thread.sh $counter $1 $2 &
	run_quiet cd $cur
	counter=$(expr $counter + 1)
done

sec=$4
sleep $sec

# clean up
# finish $4 mins fs stress testing
k_log "[end] fs-stresting testing, all done!"


