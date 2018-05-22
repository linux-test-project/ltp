#!/bin/sh

################################################################################
##                                                                            ##
## Copyright (c) 2009 FUJITSU LIMITED                                         ##
##  Author: Shi Weihua <shiwh@cn.fujitsu.com>                                 ##
## Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>                          ##
## Copyright (c) 2015-2016 Cyril Hrubis <chrubis@suse.cz>                     ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software Foundation,   ##
## Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           ##
##                                                                            ##
################################################################################

TCID="cgroup_fj_function2"
TST_TOTAL=7

. cgroup_fj_common.sh

subsystem=$1

usage_and_exit()
{
    echo "usage of cgroup_fj_function2.sh: "
    echo "  ./cgroup_fj_function2.sh subsystem"
    echo "example: ./cgroup_fj_function2.sh cpuset"

    tst_brkm TBROK "$1"
}

if [ "$#" -ne "1" ]; then
    usage_and_exit "Invalid number of parameters"
fi

# Move a task from group to group
test1()
{
    if ! attach_and_check "$pid" "$start_path/ltp_1"; then
         tst_resm TFAIL "Failed to attach task"
         return
    fi

    if ! attach_and_check "$pid" "$start_path"; then
         tst_resm TFAIL "Failed to attach task"
         return
    fi

    tst_resm TPASS "Task attached succesfully"
}

# Group can be renamed with mv
test2()
{
    create_subgroup "$start_path/ltp_2"

    if ! mv "$start_path/ltp_2" "$start_path/ltp_3"; then
        tst_resm TFAIL "Failed to move $start_path/ltp_2 to $start_path/ltp_3"
        rmdir "$start_path/ltp_2"
        return
    fi

    if ! rmdir "$start_path/ltp_3"; then
        tst_resm TFAIL "Failed to remove $start_path/ltp_3"
        return
    fi

    tst_resm TPASS "Successfully moved $start_path/ltp_2 to $start_path/ltp_3"
}

# Group can be renamed with mv unless the target name exists
test3()
{
    create_subgroup "$start_path/ltp_2"

    if mv "$start_path/ltp_2" "$start_path/ltp_1" > /dev/null 2>&1; then
        tst_resm TFAIL "Moved $start_path/ltp_2 over existing $start_path/ltp_1"
        return
    fi

    tst_resm TPASS "Failed to move $start_path/ltp_2 over existing $start_path/ltp_1"

    ROD rmdir "$start_path/ltp_2"
}

# Group with attached task cannot be removed
test4()
{
    if ! attach_and_check "$pid" "$start_path/ltp_1"; then
        tst_resm TFAIL "Failed to attach $pid to $start_path/ltp_1"
        return
    fi

    if rmdir "$start_path/ltp_1" > /dev/null 2>&1; then
        tst_resm TFAIL "Removed $start_path/ltp_1 which contains task $pid"
        create_subgroup "$start_path/ltp_1"
        return
    fi

    tst_resm TPASS "Group $start_path/ltp_1 with task $pid cannot be removed"
}

# Group with a subgroup cannot be removed
test5()
{
    create_subgroup "$start_path/ltp_1/a"

    if rmdir "$start_path/ltp_1" > /dev/null 2>&1; then
        tst_resm TFAIL "Removed $start_path/ltp_1 which contains subdir 'a'"
        return
    fi

    tst_resm TPASS "Dir $start_path/ltp_1 with subdir 'a' cannot be removed"

    ROD rmdir "$start_path/ltp_1/a"

    ROD echo "$pid" \> "$start_path/tasks"
}

# Group cannot be moved outside of hierarchy
test6()
{
    if mv "$start_path/ltp_1" "$PWD/ltp" > /dev/null 2>&1; then
        tst_resm TFAIL "Subgroup $start_path/ltp_1 outside hierarchy to $PWD/ltp"
        return
    fi

    tst_resm TPASS "Subgroup $start_path/ltp_1 cannot be moved to $PWD/ltp"
}

# Tasks file cannot be removed
test7()
{
    if rm "$start_path/ltp_1/tasks" > /dev/null 2>&1; then
        tst_resm TFAIL "Tasks file $start_path/ltp_1/tasks could be removed"
        return
    fi

    tst_resm TPASS "Tasks file $start_path/ltp_1/tasks cannot be removed"
}

# Test notify_on_release with invalid inputs
test8()
{
    if echo "-1" > "$start_path/ltp_1/notify_on_release" 2>/dev/null; then
        tst_resm TFAIL "Can write -1 to $start_path/ltp_1/notify_on_release"
        return
    fi

    if echo "ltp" > "$start_path/ltp_1/notify_on_release" 2>/dev/null; then
        tst_resm TFAIL "Can write ltp to $start_path/ltp_1/notify_on_release"
        return
    fi

    tst_resm TPASS "Cannot write invalid values to $start_path/ltp_1/notify_on_release"
}

# Test that notify_on_release can be changed
test9()
{
    local notify=$(ROD cat "$start_path/ltp_1/notify_on_release")
    local value

    if [ "$notify" -eq 0 ]; then
        value=1
    else
        value=0
    fi

    if ! echo "$value" > "$start_path/ltp_1/notify_on_release"; then
        tst_resm TFAIL "Failed to set $start_path/ltp_1/notify_on_release to $value"
        return
    fi

    ROD echo "$notify" \> "$start_path/ltp_1/notify_on_release"

    tst_resm TPASS "Set $start_path/ltp_1/notify_on_release to $value"
}

setup

cgroup_fj_proc&
pid=$!

start_path="$mount_point/ltp"

create_subgroup "$start_path"
create_subgroup "$start_path/ltp_1"

test1
test2
test3
test4
test5
test6
test7
test8
test9

ROD kill -9 $pid
wait $pid
ROD rmdir "$start_path/ltp_1"

tst_exit
