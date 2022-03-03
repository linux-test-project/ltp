#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2018-2019 ARM Ltd. All Rights Reserved.
# Copyright (c) 2022 Canonical Ltd.

. tst_test.sh

_cgroup_state=

_cgroup_check_return()
{
	local ret="$1"
	local msg="$2"

	tst_flag2mask TBROK
	[ "$ret" = "$?" ] && tst_brk TBROK "$msg"

	tst_flag2mask TCONF
	[ "$ret" = "$?" ] && tst_brk TCONF "$msg"
}

# Find mountpoint of the given controller
# USAGE: cgroup_get_mountpoint CONTROLLER
# RETURNS: Prints the mountpoint of the given controller
# Must call cgroup_require before calling
cgroup_get_mountpoint()
{
	local ctrl=$1
	local mountpoint

	[ $# -eq 0 ] && tst_brk TBROK "cgroup_get_mountpoint: controller not defined"
	[ "$_cgroup_state" = "" ] && tst_brk TBROK "cgroup_get_mountpoint: No previous state found. Forgot to call cgroup_require?"

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

	[ $# -eq 0 ] && tst_brk TBROK "cgroup_get_test_path: controller not defined"
	[ "$_cgroup_state" = "" ] && tst_brk TBROK "cgroup_get_test_path: No previous state found. Forgot to call cgroup_require?"

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

	[ $# -eq 0 ] && tst_brk TBROK "cgroup_get_version: controller not defined"
	[ "$_cgroup_state" = "" ] && tst_brk TBROK "cgroup_get_version: No previous state found. Forgot to call cgroup_require?"

	version=$(echo "$_cgroup_state" | grep -w "^$ctrl" | awk '{ print $2 }')
	[ "$version" = "" ] && tst_brk TBROK "cgroup_get_version: Could not find controller $ctrl"

	echo "$version"

	return 0
}

# Cleans up any setup done by calling cgroup_require.
# USAGE: cgroup_cleanup
# Can be safely called even when no setup has been done
cgroup_cleanup()
{
	[ "$_cgroup_state" = "" ] && return 0

	tst_cgctl cleanup "$_cgroup_state"

	_cgroup_check_return "$?" "cgroup_cleanup: tst_cgctl cleanup exited"

	_cgroup_state=""

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

	[ $# -eq 0 ] && tst_brk TBROK "cgroup_get_task_list: controller not defined"

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

	[ $# -eq 0 ] && tst_brk TBROK "cgroup_require: controller not defined"

	[ ! -f /proc/cgroups ] && tst_brk TCONF "Kernel does not support control groups"

	_cgroup_state=$(tst_cgctl require "$ctrl" $$)

	_cgroup_check_return "$?" "cgroup_require: tst_cgctl require exited"

	[ "$_cgroup_state" = "" ] && tst_brk TBROK "cgroup_require: No state was set after call to tst_cgctl require?"

	return 0
}
