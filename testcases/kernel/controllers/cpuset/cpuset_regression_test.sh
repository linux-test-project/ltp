#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
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
TST_TESTFUNC=test
TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_MIN_KVER="3.18"

. cgroup_lib.sh

LOCAL_MOUNTPOINT="cpuset_test"
BACKUP_DIRECTORY="cpuset_backup"

root_cpuset_dir=
cpu_exclusive="cpuset.cpu_exclusive"
cpus="cpuset.cpus"
old_cpu_exclusive_value=1

# cpuset_backup_and_update <backup_dir> <what> <value>
# Create backup of the values of a specific file (<what>)
# in all cpuset groups and set the value to <value>
# The backup is written to <backup_dir> in the same structure
# as in the cpuset filesystem
cpuset_backup_and_update()
{
	local backup_dir=$1
	local what=$2
	local value=$3
	local old_dir=$PWD

	cd ${root_cpuset_dir}
	find . -mindepth 2 -name ${what} -print0 |
	while IFS= read -r -d '' file; do
		mkdir -p "$(dirname "${backup_dir}/${file}")"
		cat "${file}" > "${backup_dir}/${file}"
		echo "${value}" > "${file}"
	done

	cd $old_dir
}

# cpuset_restore <backup_dir> <what>
# Restores the value of a file (<what>) in all cpuset
# groups from the backup created by cpuset_backup_and_update
cpuset_restore()
{
	local backup_dir=$1
	local what=$2
	local old_dir=$PWD

	cd ${backup_dir}
	find . -mindepth 2 -name ${what} -print0 |
	while IFS= read -r -d '' file; do
		cat "${file}" > "${root_cpuset_dir}/${file}"
	done

	cd $old_dir
}

setup()
{
	if ! is_cgroup_subsystem_available_and_enabled "cpuset"; then
		tst_brk TCONF "Either kernel does not support cpuset controller or feature not enabled"
	fi

	# We need to mount cpuset if it is not found.
	root_cpuset_dir=$(get_cgroup_mountpoint cpuset)
	if [ -z "$root_cpuset_dir" ]; then
		root_cpuset_dir="$LOCAL_MOUNTPOINT"

		ROD_SILENT mkdir -p ${root_cpuset_dir}
		ROD_SILENT mount -t cpuset cpuset ${root_cpuset_dir}
	fi

	if ! [ -f ${root_cpuset_dir}/${cpu_exclusive} ]; then
		cpu_exclusive=cpu_exclusive
		cpus=cpus
	fi

	if ! [ -f ${root_cpuset_dir}/${cpu_exclusive} ]; then
		tst_brk TBROK "Both cpuset.cpu_exclusive and cpu_exclusive do not exist"
	fi

	# Ensure that no group explicitely uses a cpu,
	# otherwise setting cpuset.cpus for the testgroup will fail
	mkdir ${BACKUP_DIRECTORY}
	cpuset_backup_and_update "${PWD}/${BACKUP_DIRECTORY}" ${cpus} ""

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

	if [ -d "$LOCAL_MOUNTPOINT" ]; then
		umount ${LOCAL_MOUNTPOINT}
		if [ $? -ne 0 ]; then
			tst_res TWARN "'umount ${LOCAL_MOUNTPOINT}' failed"
		fi

		rmdir ${LOCAL_MOUNTPOINT}
	fi
}

test()
{
	local cpu_exclusive_tmp cpus_value

	ROD_SILENT mkdir ${root_cpuset_dir}/testdir

	# Creat an exclusive cpuset.
	echo 1 > ${root_cpuset_dir}/testdir/${cpu_exclusive}
	[ $? -ne 0 ] && tst_brk TFAIL "'echo 1 > ${root_cpuset_dir}/testdir/${cpu_exclusive}' failed"

	cpu_exclusive_tmp=$(cat ${root_cpuset_dir}/testdir/${cpu_exclusive})
	if [ "${cpu_exclusive_tmp}" != "1" ]; then
		tst_brk TFAIL "${cpu_exclusive} is '${cpu_exclusive_tmp}', expected '1'"
	fi

	# This may trigger the kernel crash
	echo 0 > ${root_cpuset_dir}/testdir/${cpus}
	[ $? -ne 0 ] && tst_brk TFAIL "'echo 0 > ${root_cpuset_dir}/testdir/${cpus}' failed"

	cpus_value=$(cat ${root_cpuset_dir}/testdir/${cpus})
	if [ "${cpus_value}" != "0" ]; then
		tst_brk TFAIL "${cpus} is '${cpus_value}', expected '0'"
	fi

	tst_res TPASS "Bug is not reproducible"
}

tst_run
