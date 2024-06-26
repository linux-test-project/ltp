#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 FUJITSU LIMITED
# Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
# Copyright (c) 2015-2016 Cyril Hrubis <chrubis@suse.cz>
# Author: Shi Weihua <shiwh@cn.fujitsu.com>

for arg; do
    TCID="${TCID}_$arg"
done

TST_NEEDS_CMDS="rmdir killall"
TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1

attach_and_check()
{
    local pid="$1"
    local path="$2"
    local task
    shift

    tst_res TINFO "Attaching task $pid to $path"

    ROD echo "$pid" \> "$path/$task_list"

    for task in $(cat "$path/$task_list"); do
        if [ "$task" -ne "$pid" ]; then
            tst_res TINFO "Unexpected pid $task in $path/$task_list, expected $pid"
            return 1
        fi
    done

    return 0
}

create_subgroup()
{
    path="$1"

    [ ! -d "$path" ] && ROD mkdir "$path"

    # cpuset.cpus and cpuset.mems must be initialized with suitable value
    # before any pids are attached.
    # Only needs to be done for cgroup v1 as sets are inherited from parents
    # by default in cgroup v2.
    if [ "$cgroup_version" = "1" ] && [ "$subsystem" = "cpuset" ]; then
        if [ -e "$mount_point/cpus" ]; then
            ROD cat "$mount_point/cpus" \> "$path/cpus"
            ROD cat "$mount_point/mems" \> "$path/mems"
        else
            ROD cat "$mount_point/cpuset.cpus" \> "$path/cpuset.cpus"
            ROD cat "$mount_point/cpuset.mems" \> "$path/cpuset.mems"
        fi
    fi
}

common_setup()
{
    cgroup_require "$subsystem"
    mount_point=$(cgroup_get_mountpoint "$subsystem")
    start_path=$(cgroup_get_test_path "$subsystem")
    cgroup_version=$(cgroup_get_version "$subsystem")
    task_list=$(cgroup_get_task_list "$subsystem")

    [ "$cgroup_version" = "2" ] && ROD echo "+$subsystem" \> "$start_path/cgroup.subtree_control"
    tst_res TINFO "test starts with cgroup version $cgroup_version"
}

common_cleanup()
{
    killall -9 cgroup_fj_proc >/dev/null 2>&1

    tst_res TINFO "Removing all ltp subgroups..."

    [ -d "$start_path" ] && find "$start_path" -depth -type d -exec rmdir '{}' \;

    cgroup_cleanup

    [ "$cgroup_version" = "2" ] && ROD echo "-$subsystem" \> "/sys/fs/cgroup/cgroup.subtree_control"
}

. cgroup_lib.sh
