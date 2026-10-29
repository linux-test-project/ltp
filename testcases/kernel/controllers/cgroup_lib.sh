#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019-2022 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2018-2019 ARM Ltd. All Rights Reserved.
# Copyright (c) 2022 Canonical Ltd.

_cgroup_state=

# Find mountpoint of the given controller
# USAGE: cgroup_get_mountpoint CONTROLLER
# RETURNS: Prints the mountpoint of the given controller
# Must call cgroup_require before calling
cgroup_get_mountpoint()
{
	local ctrl="$1"
	local mountpoint

	[ "$ctrl" ] || tst_brk TBROK "cgroup_get_mountpoint: controller not defined"
	[ "$_cgroup_state" ] || tst_brk TBROK "cgroup_get_mountpoint: No previous state found. Forgot to call cgroup_require?"

	mountpoint=$(echo "$_cgroup_state" | grep -w "^$ctrl" | awk '{ print $4 }')
	echo "$mountpoint"

	return 0
}

# Get the test path of a given controller that has been created by the cgroup C API
# USAGE: cgroup_get_test_path CONTROLLER
# RETURNS: Prints the path to the test direcory
# Must call cgroup_require before calling
cgroup_get_test_path()
{
	local ctrl="$1"
	local mountpoint
	local test_path

	[ "$ctrl" ] || tst_brk TBROK "cgroup_get_test_path: controller not defined"
	[ "$_cgroup_state" ] || tst_brk TBROK "cgroup_get_test_path: No previous state found. Forgot to call cgroup_require?"

	mountpoint=$(cgroup_get_mountpoint "$ctrl")

	test_path="$mountpoint/ltp/test-$$"

	[ ! -e "$test_path" ] && tst_brk TBROK "cgroup_get_test_path: No test path found. Forgot to call cgroup_require?"

	echo "$test_path"

	return 0
}

# Gets the cgroup version of the given controller
# USAGE: cgroup_get_version CONTROLLER
# RETURNS: "1" if version 1 and "2" if version 2
# Must call cgroup_require before calling
cgroup_get_version()
{
	local ctrl="$1"
	local version

	[ "$ctrl" ] || tst_brk TBROK "cgroup_get_version: controller not defined"
	[ "$_cgroup_state" ] || tst_brk TBROK "cgroup_get_version: No previous state found. Forgot to call cgroup_require?"

	version=$(echo "$_cgroup_state" | grep -w "^$ctrl" | awk '{ print $2 }')
	[  "$version" ] || tst_brk TBROK "cgroup_get_version: Could not find controller $ctrl"

	echo "$version"

	return 0
}

# Cleans up any setup done by calling cgroup_require.
# USAGE: cgroup_cleanup
# Can be safely called even when no setup has been done
cgroup_cleanup()
{
	[ "$_cgroup_state" ] || return 0

	ROD tst_cgctl cleanup "$_cgroup_state"

	_cgroup_state=

	return 0
}

# Get the task list of the given controller
# USAGE: cgroup_get_task_list CONTROLLER
# RETURNS: prints out "cgroup.procs" if version 2 otherwise "tasks"
# Must call cgroup_require before calling
cgroup_get_task_list()
{
	local ctrl="$1"
	local version

	[ "$ctrl" ] || tst_brk TBROK "cgroup_get_task_list: controller not defined"

	version=$(cgroup_get_version "$ctrl")

	if [ "$version" = "2" ]; then
		echo "cgroup.procs"
	else
		echo "tasks"
	fi

	return 0
}

# Mounts and configures the given controller
# USAGE: cgroup_require CONTROLLER
cgroup_require()
{
	local ctrl="$1"
	local ret

	[ "$ctrl" ] || tst_brk TBROK "cgroup_require: controller not defined"

	[ ! -f /proc/cgroups ] && tst_brk TCONF "Kernel does not support control groups"

	_cgroup_state=$(tst_cgctl require "$ctrl" $$)
	ret=$?

	if [ $ret -eq 32 ]; then
		tst_brk TCONF "'tst_cgctl require $ctrl' failed. $ctrl controller not available?"
		return $ret
	fi

	if [ $ret -ne 0 ]; then
		tst_brk TBROK "'tst_cgctl require $ctrl' failed. LTP missing $ctrl controller support?"
		return $ret
	fi

	[ "$_cgroup_state" ] || tst_brk TBROK "cgroup_require: No state was set after call to tst_cgctl require?"

	return 0
}

. tst_test.sh
