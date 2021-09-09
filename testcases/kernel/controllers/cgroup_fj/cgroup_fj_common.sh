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

for arg; do
    TCID="${TCID}_$arg"
done

. test.sh

exist_subsystem()
{
    local subsystem="$1"
    local exist=`grep -w $subsystem /proc/cgroups | cut -f1`

    if [ -z "$exist" ]; then
        tst_brkm TCONF "Subsystem $subsystem not supported"
    fi
}

attach_and_check()
{
    local pid="$1"
    local path="$2"
    local task
    shift

    tst_resm TINFO "Attaching task $pid to $path"

    ROD echo "$pid" \> "$path/tasks"

    for task in $(cat "$path/tasks"); do
        if [ "$task" -ne "$pid" ]; then
            tst_resm TINFO "Unexpected pid $task in $path/tasks, expected $pid"
            return 1
        fi
    done

    return 0
}

create_subgroup()
{
    path="$1"

    ROD mkdir "$path"

    # cpuset.cpus and cpuset.mems must be initialized with suitable value
    # before any pids are attached
    if [ "$subsystem" = "cpuset" ]; then
        if [ -e "$mount_point/cpus" ]; then
            ROD cat "$mount_point/cpus" \> "$path/cpus"
            ROD cat "$mount_point/mems" \> "$path/mems"
        else
            ROD cat "$mount_point/cpuset.cpus" \> "$path/cpuset.cpus"
            ROD cat "$mount_point/cpuset.mems" \> "$path/cpuset.mems"
        fi
    fi
}


setup()
{
    tst_require_root
    tst_require_cmds killall

    if [ ! -f /proc/cgroups ]; then
        tst_brkm TCONF "Kernel does not support for control groups"
    fi

    exist_subsystem "$subsystem"

    tst_tmpdir
    TST_CLEANUP=cleanup

    mount_point=`grep -w $subsystem /proc/mounts | grep -w "cgroup" | \
	cut -f 2 | cut -d " " -f2`

    if [ -z "$mount_point" ]; then
        try_umount=1
        mount_point="/dev/cgroup"
	tst_resm TINFO "Subsystem $subsystem is not mounted, mounting it at $mount_point"
        ROD mkdir $mount_point
        ROD mount -t cgroup -o "$subsystem" "ltp_cgroup" "$mount_point"
    else
	tst_resm TINFO "Subsystem $subsystem is mounted at $mount_point"
    fi
}

cleanup()
{
    tst_rmdir

    killall -9 cgroup_fj_proc >/dev/null 2>&1

    tst_resm TINFO "Removing all ltp subgroups..."

    find "$mount_point/ltp/" -depth -type d -exec rmdir '{}' \;

    if [ -z "$try_umount" ]; then
	return
    fi

    if grep -q "$mount_point" /proc/mounts; then
        EXPECT_PASS umount "$mount_point"
    fi

    if [ -e "$mount_point" ]; then
        EXPECT_PASS rmdir "$mount_point"
    fi
}
