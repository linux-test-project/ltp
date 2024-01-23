#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2016-2022
# Copyright (c) 2015 Fujitsu Ltd.
# Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
#
# This is a regression test for commit:
# http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/?id=bb2bc55
#
# A newly created cpuset group crashed the kernel, if exclusive was set to 1,
# before a cpuset was set.

TST_SETUP=setup
TST_CLEANUP=cleanup
TST_TESTFUNC=do_test
TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_MIN_KVER="3.18"

LOCAL_MOUNTPOINT="cpuset_test"
BACKUP_DIRECTORY="cpuset_backup"

cpu_num=
root_cpuset_dir=
cpu_exclusive="cpuset.cpu_exclusive"
cpus="cpuset.cpus"
old_cpu_exclusive_value=1

# Check if there are cpuset groups
cpuset_has_groups()
{
	find ${root_cpuset_dir} -mindepth 1 -type d -exec echo 1 \; -quit
}

# cpuset_find_parent_first <what>
# Do a parent first find of <what>
cpuset_find_parent_first()
{
	local what=$1

	find . -mindepth 2 -name ${what} |
	    awk '{print length($0) " " $0}' |
	    sort -n | cut -d " " -f 2-
}

# cpuset_find_child_first <what>
# Do a child first find of <what>
cpuset_find_child_first()
{
	local what=$1

	find . -mindepth 2 -name ${what} |
	    awk '{print length($0) " " $0}' |
	    sort -nr | cut -d " " -f 2-
}

# cpuset_backup_and_update <backup_dir> <what>
# Create backup of the values of a specific file (<what>)
# in all cpuset groups and set the value to 1
# The backup is written to <backup_dir> in the same structure
# as in the cpuset filesystem
cpuset_backup_and_update()
{
	local backup_dir=$1
	local what=$2
	local old_dir=$PWD
	local cpu_max=$((cpu_num - 1))
	local res

	cd ${root_cpuset_dir}


	cpuset_find_parent_first ${what} |
	while read -r file; do
		[ "$(cat "${file}")" = "" ] && continue

		mkdir -p "$(dirname "${backup_dir}/${file}")"
		cat "${file}" > "${backup_dir}/${file}"
		echo "0-$cpu_max" > "${file}" || exit 1
	done
	if [ $? -ne 0 ]; then
		cd $old_dir
		return 1
	fi

	cpuset_find_child_first ${what} |
	while read -r file; do
		[ "$(cat "${file}")" = "" ] && continue
		echo "1" > "${file}"
	done
	res=$?

	cd $old_dir

	return $res
}

# cpuset_restore <backup_dir> <what>
# Restores the value of a file (<what>) in all cpuset
# groups from the backup created by cpuset_backup_and_update
cpuset_restore()
{
	local backup_dir=$1
	local what=$2
	local old_dir=$PWD
	local cpu_max=$((cpu_num - 1))

	cd ${backup_dir}

	cpuset_find_parent_first ${what} |
	while read -r file; do
		echo "0-$cpu_max" > "${root_cpuset_dir}/${file}"
	done

	cpuset_find_child_first ${what} |
	while read -r file; do
		cat "${file}" > "${root_cpuset_dir}/${file}"
	done

	cd $old_dir
}

setup()
{
	cgroup_require "cpuset"
	cgroup_version=$(cgroup_get_version "cpuset")
	root_cpuset_dir=$(cgroup_get_mountpoint "cpuset")
	testpath=$(cgroup_get_test_path "cpuset")
	task_list=$(cgroup_get_task_list "cpuset")

	tst_res TINFO "test starts with cgroup version $cgroup_version"

	if [ "$cgroup_version" = "2" ]; then
		tst_brk TCONF "cgroup v2 found, skipping test"
		return
	fi

	if ! [ -f ${root_cpuset_dir}/${cpu_exclusive} ]; then
		cpu_exclusive=cpu_exclusive
		cpus=cpus
	fi

	if ! [ -f ${root_cpuset_dir}/${cpu_exclusive} ]; then
		tst_brk TBROK "Both cpuset.cpu_exclusive and cpu_exclusive do not exist"
	fi

	# Ensure that we can use cpu 0 exclusively
	if [ "$(cpuset_has_groups)" = "1" ]; then
		cpu_num=$(tst_getconf _NPROCESSORS_ONLN)
		if [ $cpu_num -lt 2 ]; then
			tst_brk TCONF "There are already cpuset groups, so at least two cpus are required."
		fi

		# Use cpu 1 for all existing cpuset cgroups
		mkdir ${BACKUP_DIRECTORY}
		cpuset_backup_and_update "${PWD}/${BACKUP_DIRECTORY}" ${cpus}
		[ $? -ne 0 ] && tst_brk TBROK "Unable to prepare existing cpuset cgroups"
	fi

	old_cpu_exclusive_value=$(cat ${root_cpuset_dir}/${cpu_exclusive})
	if [ "${old_cpu_exclusive_value}" != "1" ];then
		echo 1 > ${root_cpuset_dir}/${cpu_exclusive}
		[ $? -ne 0 ] && tst_brk TBROK "'echo 1 > ${root_cpuset_dir}/${cpu_exclusive}' failed"
	fi
}

cleanup()
{
	if [ -d "${root_cpuset_dir}/testdir" ]; then
		rmdir ${root_cpuset_dir}/testdir
	fi

	if [ -d "${BACKUP_DIRECTORY}" ]; then
		cpuset_restore "${PWD}/${BACKUP_DIRECTORY}" ${cpus}
	fi

	if [ "$old_cpu_exclusive_value" != 1 ]; then
		# Need to flush, or write may fail with: "Device or resource busy"
		sync
		echo ${old_cpu_exclusive_value} > ${root_cpuset_dir}/${cpu_exclusive}
	fi

	cgroup_cleanup
}

do_test()
{
	local cpu_exclusive_tmp cpus_value

	ROD_SILENT mkdir ${root_cpuset_dir}/testdir

	# Creat an exclusive cpuset.
	if ! echo 1 > ${root_cpuset_dir}/testdir/${cpu_exclusive}; then
		tst_res TFAIL "'echo 1 > ${root_cpuset_dir}/testdir/${cpu_exclusive}' failed"
		return
	fi

	cpu_exclusive_tmp=$(cat ${root_cpuset_dir}/testdir/${cpu_exclusive})
	if [ "${cpu_exclusive_tmp}" != "1" ]; then
		tst_res TFAIL "${cpu_exclusive} is '${cpu_exclusive_tmp}', expected '1'"
		return
	fi

	# This may trigger the kernel crash
	if ! echo 0 > ${root_cpuset_dir}/testdir/${cpus}; then
		tst_res TFAIL "'echo 0 > ${root_cpuset_dir}/testdir/${cpus}' failed"
		return
	fi

	cpus_value=$(cat ${root_cpuset_dir}/testdir/${cpus})
	if [ "${cpus_value}" != "0" ]; then
		tst_res TFAIL "${cpus} is '${cpus_value}', expected '0'"
		return
	fi

	tst_res TPASS "Bug is not reproducible"
}

. cgroup_lib.sh
tst_run
