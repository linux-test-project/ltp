#/bin/bash
#
# Test thread for File system metadata stress testing script
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

# run program and do not display the output
function run_quiet()
{
	local cmd=$*
	$cmd >/dev/null 2>&1
	return $?
}

function k_log()
{
	echo [$(date "+%m-%d %H:%M:%S")] $* | tee -a $K_LOG 
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

# Compare 2 trees, if it is the same, return 0, or return 1
#
# we need to make sure whether the tree has changes after we 
# finish a lot of meta operations on he heavy workloads, this 
# function can compare the hierarchy between 2 trees.
#
# the basic idea is diff the output by command find.

function k_tree_diff()
{
	local ta=$1 # tree a
	local tb=$2 # tree b
	
	local md5a=$(run_quiet cd $ta; find | md5sum | awk '{ print $1}')
	local md5b=$(run_quiet cd $tb; find | md5sum | awk '{ print $1}')

	if [ $md5a = $md5b ];then
		 return 0
	else
		 return 1
	fi
}


dir=$1 
depth=$2
width=$3 
result=

k_log "thread $1 starts with pid $$"
echo $$ | tee -a $K_THREADS_PID
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
	result=pass
	k_tree_diff $dir $new_dir || result=fail
	k_result "thread $dir: $result to compare result between dir $dir and $new_dir" 

	rm $new_dir -fr
done
run_quiet cd $cwd 

# test ends, remove the oringal tree
rm $dir -fr
