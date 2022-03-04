#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 FUJITSU LIMITED
# Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
# Copyright (c) 2015-2016 Cyril Hrubis <chrubis@suse.cz>
# Author: Shi Weihua <shiwh@cn.fujitsu.com>

TCID="cgroup_fj_function2"
TST_TESTFUNC=test
TST_SETUP=setup
TST_CLEANUP=cleanup
TST_CNT=9
TST_POS_ARGS=1

. cgroup_fj_common.sh

subsystem=$1

usage_and_exit()
{
    echo "usage of cgroup_fj_function2.sh: "
    echo "  ./cgroup_fj_function2.sh subsystem"
    echo "example: ./cgroup_fj_function2.sh cpuset"

    tst_brk TBROK "$1"
}

if [ "$#" -ne "1" ]; then
    usage_and_exit "Invalid number of parameters"
fi

# Move a task from group to group
test1()
{
    # mv'ing cgroups is not available in cgroup2
    if [ "$cgroup_version" = "2" ]; then
        tst_res TCONF "Controller mounted on cgroup2 hierachy, skipping test"
        return
    fi

    if ! attach_and_check "$pid" "$start_path/ltp_1"; then
         tst_res TFAIL "Failed to attach task"
         return
    fi

    if ! attach_and_check "$pid" "$start_path"; then
         tst_res TFAIL "Failed to attach task"
         return
    fi

    tst_res TPASS "Task attached succesfully"
}

# Group can be renamed with mv
test2()
{
    # mv'ing cgroups is not available in cgroup2
    if [ "$cgroup_version" = "2" ]; then
        tst_res TCONF "Controller mounted on cgroup2 hierachy, skipping test"
        return
    fi

    create_subgroup "$start_path/ltp_2"

    if ! mv "$start_path/ltp_2" "$start_path/ltp_3"; then
        tst_res TFAIL "Failed to move $start_path/ltp_2 to $start_path/ltp_3"
        rmdir "$start_path/ltp_2"
        return
    fi

    if ! rmdir "$start_path/ltp_3"; then
        tst_res TFAIL "Failed to remove $start_path/ltp_3"
        return
    fi

    tst_res TPASS "Successfully moved $start_path/ltp_2 to $start_path/ltp_3"
}

# Group can be renamed with mv unless the target name exists
test3()
{
    # mv'ing cgroups is not available in cgroup2
    if [ "$cgroup_version" = "2" ]; then
        tst_res TCONF "Controller mounted on cgroup2 hierachy, skipping test"
        return
    fi

    create_subgroup "$start_path/ltp_2"

    if mv "$start_path/ltp_2" "$start_path/ltp_1" > /dev/null 2>&1; then
        tst_res TFAIL "Moved $start_path/ltp_2 over existing $start_path/ltp_1"
        return
    fi

    tst_res TPASS "Failed to move $start_path/ltp_2 over existing $start_path/ltp_1"

    ROD rmdir "$start_path/ltp_2"
}

# Group with attached task cannot be removed
test4()
{
    if ! attach_and_check "$pid" "$start_path/ltp_1"; then
        tst_res TFAIL "Failed to attach $pid to $start_path/ltp_1"
        return
    fi

    if rmdir "$start_path/ltp_1" > /dev/null 2>&1; then
        tst_res TFAIL "Removed $start_path/ltp_1 which contains task $pid"
        return
    fi

    tst_res TPASS "Group $start_path/ltp_1 with task $pid cannot be removed"
}

# Group with a subgroup cannot be removed
test5()
{
    # We need to move the tasks back to root to create a subgroup
    if [ "$cgroup_version" = "2" ]; then
        for pid in $(cat "$start_path/ltp_1/$task_list"); do
		    echo $pid > "$mount_point/$task_list" 2> /dev/null
	    done

        ROD echo "+$subsystem" \> "$start_path/ltp_1/cgroup.subtree_control"
    fi

    create_subgroup "$start_path/ltp_1/a"

    if rmdir "$start_path/ltp_1" > /dev/null 2>&1; then
        tst_res TFAIL "Removed $start_path/ltp_1 which contains subdir 'a'"
        return
    fi

    tst_res TPASS "Dir $start_path/ltp_1 with subdir 'a' cannot be removed"

    ROD rmdir "$start_path/ltp_1/a"

    [ "$cgroup_version" = "2" ] && ROD echo "-$subsystem" \> "$start_path/ltp_1/cgroup.subtree_control"
    ROD echo "$pid" \> "$start_path/ltp_1/$task_list"
}

# Group cannot be moved outside of hierarchy
test6()
{
    # mv'ing cgroups is not available in cgroup2
    if [ "$cgroup_version" = "2" ]; then
        tst_res TCONF "Controller mounted on cgroup2 hierachy, skipping test"
        return
    fi

    if mv "$start_path/ltp_1" "$PWD/ltp" > /dev/null 2>&1; then
        tst_res TFAIL "Subgroup $start_path/ltp_1 outside hierarchy to $PWD/ltp"
        return
    fi

    tst_res TPASS "Subgroup $start_path/ltp_1 cannot be moved to $PWD/ltp"
}

# Tasks file cannot be removed
test7()
{
    if rm "$start_path/ltp_1/$task_list" > /dev/null 2>&1; then
        tst_res TFAIL "Tasks file $start_path/ltp_1/$task_list could be removed"
        return
    fi

    tst_res TPASS "Tasks file $start_path/ltp_1/tasks cannot be removed"
}

# Test notify_on_release with invalid inputs
test8()
{
    # notify_on_release is not available in cgroup2 so skip the test
    if [ "$cgroup_version" = "2" ]; then
        tst_res TCONF "Controller mounted on cgroup2 hierachy, skipping test"
        return
    fi

    if echo "-1" > "$start_path/ltp_1/notify_on_release" 2>/dev/null; then
        tst_res TFAIL "Can write -1 to $start_path/ltp_1/notify_on_release"
        return
    fi

    if echo "ltp" > "$start_path/ltp_1/notify_on_release" 2>/dev/null; then
        tst_res TFAIL "Can write ltp to $start_path/ltp_1/notify_on_release"
        return
    fi

    tst_res TPASS "Cannot write invalid values to $start_path/ltp_1/notify_on_release"
}

# Test that notify_on_release can be changed
test9()
{
    # notify_on_release is not available in cgroup2 so skip the test
    if [ "$cgroup_version" = "2" ]; then
        tst_res TCONF "Controller mounted on cgroup2 hierachy, skipping test"
        return
    fi

    local notify=$(ROD cat "$start_path/ltp_1/notify_on_release")
    local value

    if [ "$notify" -eq 0 ]; then
        value=1
    else
        value=0
    fi

    if ! echo "$value" > "$start_path/ltp_1/notify_on_release"; then
        tst_res TFAIL "Failed to set $start_path/ltp_1/notify_on_release to $value"
        return
    fi

    ROD echo "$notify" \> "$start_path/ltp_1/notify_on_release"

    tst_res TPASS "Set $start_path/ltp_1/notify_on_release to $value"
}

setup()
{
    common_setup
    cgroup_fj_proc&
    pid=$!
    create_subgroup "$start_path/ltp_1"
}

cleanup()
{
    kill -9 $pid >/dev/null 2>&1
    wait $pid >/dev/null 2>&1
    rmdir "$start_path/ltp_1" >/dev/null 2>&1
    common_cleanup
}

tst_run
