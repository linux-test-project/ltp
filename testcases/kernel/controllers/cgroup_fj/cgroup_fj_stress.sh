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

TCID="cgroup_fj_stress"
TST_TOTAL=1

. cgroup_fj_common.sh

subsystem="$1"
subgroup_num="$2"
subgroup_depth="$3"
attach_operation="$4"

usage_and_exit()
{
    echo "usage of cgroup_fj_stress.sh: "
    echo "  ./cgroup_fj_stress.sh subsystem subgroup_num subgroup_depth attach_operation"
    echo "    subgroup_num"
    echo "      number of subgroups created in group"
    echo "    subgroup_depth"
    echo "      depth of the created tree"
    echo "    attach_operation"
    echo "      none - do not attach anything"
    echo "      one  - move one processe around"
    echo "      each - attach process to each subgroup"
    echo "example: ./cgroup_fj_stress.sh cpuset 1 1 one"
    echo
    tst_brkm TBROK "$1"
}

if [ "$#" -ne "4" ]; then
    usage_and_exit "Wrong number of parameters, expected 4"
fi

case $subgroup_num in
    ''|*[!0-9]*) usage_and_exit "Number of subgroups must be possitive integer";;
    *) ;;
esac

case $subgroup_depth in
    ''|*[!0-9]*) usage_and_exit "Depth of the subgroup tree must be possitive integer";;
    *) ;;
esac

case $attach_operation in
    'none'|'one'|'each');;
    *) usage_and_exit "Invalid attach operation: $attach_operation";;
esac

setup

export TMPFILE=./tmp_tasks.$$

count=0
collected_pids=

build_subgroups()
{
    local cur_path="$1"
    local cur_depth="$2"
    local i

    if [ "$cur_depth" -gt "$subgroup_depth" ]; then
        return
    fi

    create_subgroup "$cur_path"
    count=$((count+1))

    for i in $(seq 1 $subgroup_num); do
         build_subgroups "$cur_path/$i" $((cur_depth+1))
    done
}

attach_task()
{
    local cur_path="$1"
    local cur_depth="$2"
    local ppid="$3"
    local i

    if [ "$cur_depth" -gt "$subgroup_depth" ]; then
        return
    fi

    if [ -z "$ppid" ]; then
        cgroup_fj_proc&
        pid=$!
        collected_pids="$collected_pids $pid"
    else
        pid="$ppid"
    fi

    if ! attach_and_check "$pid" "$cur_path"; then
            fail=1
    fi

    for i in $(seq 1 $subgroup_num); do
         local new_path="$cur_path/$i"
         attach_task "$new_path" $((cur_depth+1)) "$ppid"
    done

    if [ -n "$ppid" ]; then
        if ! attach_and_check "$pid" "$cur_path"; then
            fail=1
        fi
    fi
}

start_path="$mount_point/ltp"

tst_resm TINFO "Creating subgroups ..."

build_subgroups "$start_path" 0

tst_resm TINFO "... mkdired $count times"

case $attach_operation in
"one" )
    cgroup_fj_proc &
    pid=$!

    tst_resm TINFO "Moving one task around"
    attach_task "$start_path" 0 "$pid"
    ROD kill -9 "$pid"
    wait "$pid"
    ;;
"each" )
    tst_resm TINFO "Attaching task to each subgroup"
    attach_task "$start_path" 0
    for pid in $collected_pids; do
        ROD kill -9 "$pid"
        wait "$pid"
    done
    ;;
*  )
    ;;
esac

if [ -n "$fail" ]; then
    tst_resm TFAIL "Attaching tasks failed!"
else
    tst_resm TPASS "All done!"
fi

tst_exit
