#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019-2022 Petr Vorel <pvorel@suse.cz>
# Copyright (c) 2018-2019 ARM Ltd. All Rights Reserved.

# Find mountpoint to given subsystem
# get_cgroup_mountpoint SUBSYSTEM
# RETURN: 0 if mountpoint found, otherwise 1
get_cgroup_mountpoint()
{
	local subsystem=$1
	local mntpoint

	[ $# -eq 0 ] && tst_brk TBROK "get_cgroup_mountpoint: subsystem not defined"

	mntpoint=$(grep cgroup /proc/mounts | grep -w $subsystem | awk '{ print $2 }')
	[ -z "$mntpoint" ] && return 1

	echo $mntpoint
	return 0
}

# Check if given subsystem is supported and enabled
# is_cgroup_subsystem_available_and_enabled SUBSYSTEM
# RETURN: 0 if subsystem supported and enabled, otherwise 1
is_cgroup_subsystem_available_and_enabled()
{
	local val
	local subsystem=$1

	[ $# -eq 0 ] && tst_brk TBROK "is_cgroup_subsystem_available_and_enabled: subsystem not defined"

	val=$(grep -w $subsystem /proc/cgroups | awk '{ print $4 }')
	[ "$val" = "1" ] && return 0

	return 1
}

. tst_test.sh
