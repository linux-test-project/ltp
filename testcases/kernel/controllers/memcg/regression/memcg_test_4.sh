#! /bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 FUJITSU LIMITED
# Author: Li Zefan <lizf@cn.fujitsu.com>

cgroup_version="$1"
mount_point="$2"
test_path="$3"

if [ "$cgroup_version" = "2" ]; then
	task_list="cgroup.procs"
	memory_limit="memory.max"
else
	task_list="tasks"
	memory_limit="memory.limit_in_bytes"
fi

echo $$ > "$test_path/$task_list"

./memcg_test_4 &
pid=$!
sleep 1

# let $pid allocate 100M memory
/bin/kill -SIGUSR1 $pid
sleep 1

# shrink memory, and then 80M will be swapped
echo 40M > "$test_path/$memory_limit"

# turn off swap, and swapoff will be killed
swapoff -a
sleep 1
echo $pid > "$mount_point/$task_list" 2> /dev/null
echo $$ > "$mount_point/$task_list"  2> /dev/null

# now remove the cgroup
rmdir "$test_path"
