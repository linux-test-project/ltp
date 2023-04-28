#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 FUJITSU LIMITED
# Copyright (c) 2015 Cedric Hnyda <chnyda@suse.com>
# Copyright (c) 2015-2016 Cyril Hrubis <chrubis@suse.cz>
# Author: Shi Weihua <shiwh@cn.fujitsu.com>

TCID="cgroup_fj_stress"
TST_CNT=1
TST_TESTFUNC=do_test
TST_SETUP=setup
TST_CLEANUP=cleanup
TST_POS_ARGS=4

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
    tst_brk TBROK "$1"
}

build_subgroups()
{
    local cur_path="$1"
    local cur_depth="$2"
    local i

    if [ "$cur_depth" -gt "$subgroup_depth" ]; then
        return
    fi

    create_subgroup "$cur_path"

    # We can only attach processes to the leaves of the tree in cgroup v2 which
    # means we need to enable the controllers everywhere inbetween.
    if [ "$cgroup_version" = "2" ] && [ "$cur_depth" -ne "$subgroup_depth" ]; then
        ROD echo "+$subsystem" \> "$cur_path/cgroup.subtree_control"
    fi
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

    if [ "$cgroup_version" = "2" ] && [ $cur_depth -eq $subgroup_depth ] || [ "$cgroup_version" = "1" ]; then
        if ! attach_and_check "$pid" "$cur_path"; then
            fail=1
        fi
    fi

    for i in $(seq 1 $subgroup_num); do
         local new_path="$cur_path/$i"
         attach_task "$new_path" $((cur_depth+1)) "$ppid"
    done

    if [ -n "$ppid" ]; then
        if [ "$cgroup_version" = "2" ] && [ $cur_depth -eq $subgroup_depth ] || [ "$cgroup_version" = "1" ]; then
            if ! attach_and_check "$pid" "$cur_path"; then
                fail=1
            fi
        fi
    fi
}

setup()
{
    export TMPFILE=./tmp_tasks.$$
    count=0
    collected_pids=

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

    common_setup
}

cleanup()
{
    common_cleanup
}

do_test()
{
    tst_res TINFO "Creating subgroups ..."

    build_subgroups "$start_path" 0

    tst_res TINFO "... mkdired $count times"

    case $attach_operation in
    "one" )
        cgroup_fj_proc &
        pid=$!

        tst_res TINFO "Moving one task around"
        attach_task "$start_path" 0 "$pid"
        ROD kill -9 "$pid"
        wait "$pid"
        ;;
    "each" )
        tst_res TINFO "Attaching task to each subgroup"
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
        tst_res TFAIL "Attaching tasks failed!"
    else
        tst_res TPASS "All done!"
    fi
}

. cgroup_fj_common.sh
tst_run
