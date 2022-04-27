#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2017 FUJITSU LIMITED. All rights reserved.
# Author: Xiao Yang <yangx.jy@cn.fujitsu.com>
#
# Test unshare command with some basic options.
# 1) If we run unshare with "--user", UID in the newly created user namespace
#    is set to 65534.
# 2) If we run unshare with "--user", GID in the newly created user namespace
#    is set to 65534.
# 3) If we run with "--user --map-root-user", UID in the newly created user
#    namespace is set to 0.
# 4) If we run with "--user --map-root-user", GID in the newly created user
#    is set to 0.
# 5) If we run with "--mount", mount and unmount events do not propagate to
#    its parent mount namespace.
# 6) If we run with "--mount --propagation shared", mount and unmount events
#    propagate to its parent mount namespace.
# 7) If we run with "--user --map-root-user --mount", mount and unmount events
#    do not propagate to its parent mount namespace.
# 8) Even if we run with "--user --map-root-user --mount --propagation shared",
#    mount and unmount events do not propagate to its parent mount namespace
#    because the shared mount is reduced to a slave mount.
#
#    Please see the following URL for detailed information:
#    http://man7.org/linux/man-pages/man7/user_namespaces.7.html
#    http://man7.org/linux/man-pages/man7/mount_namespaces.7.html

TST_CNT=8
TST_SETUP=setup
TST_CLEANUP=cleanup
TST_TESTFUNC=do_test
TST_NEEDS_ROOT=1
TST_NEEDS_TMPDIR=1
TST_NEEDS_CMDS="unshare id mount umount"

max_userns_path="/proc/sys/user/max_user_namespaces"
max_mntns_path="/proc/sys/user/max_mnt_namespaces"
default_max_userns=-1
default_max_mntns=-1

setup()
{
	# On some distributions(e.g RHEL7.4), the default value of
	# max_user_namespaces or max_mnt_namespaces is set to 0.
	# We need to change the default value to run unshare command.
	if [ -f "${max_userns_path}" ]; then
		default_max_userns=$(cat "${max_userns_path}")
		echo 1024 > "${max_userns_path}"
	fi

	if [ -f "${max_mntns_path}" ]; then
		default_max_mntns=$(cat "${max_mntns_path}")
		echo 1024 > "${max_mntns_path}"
	fi

	mkdir -p dir_A dir_B
	touch dir_A/A dir_B/B
}

cleanup()
{
	# Restore the default value to 0.
	[ ${default_max_userns} -ne -1 ] && \
		echo ${default_max_userns} > "${max_userns_path}"
	[ ${default_max_mntns} -ne -1 ] && \
		echo ${default_max_mntns} > "${max_mntns_path}"
}

check_id()
{
	local act_id="$1"
	local exp_id="$2"
	local cmd="$3"

	if [ ${act_id} -ne ${exp_id} ]; then
		tst_res TFAIL "$cmd got wrong uid/gid"
	else
		tst_res TPASS "$cmd got correct uid/gid"
	fi
}

check_mount()
{
	local tst_dir="$1"
	local exp_stat="$2"
	local cmd="$3"

	case ${exp_stat} in
	unmounted)
		if ls "${tst_dir}" | grep -qw 'A'; then
			tst_res TFAIL "$cmd got bind info"
			umount ${tst_dir}
			return
		fi
		;;
	mounted)
		if ! ls "${tst_dir}" | grep -qw 'A'; then
			tst_res TFAIL "$cmd did not get bind info"
			return
		fi
		umount ${tst_dir}
		;;
	esac

	tst_res TPASS "$cmd got bind info as expected"
}

unshare_test()
{
	local unshare_opts="$1"
	local verify_cmd="$2"
	local exp_result="$3"

	local unshare_cmd="unshare ${unshare_opts} ${verify_cmd}"

	eval ${unshare_cmd} > temp 2>&1
	if [ $? -ne 0 ]; then
		# unrecognized option or invalid option is returned if the
		# option is not supported by unshare command(e.g. RHEL6).
		# Invalid argument or Operation not permitted is returned
		# if the feature is not supported by kernel(e.g. RHEL7).
		grep -q -E "unrecognized option|invalid option|Invalid argument|Operation not permitted" temp
		if [ $? -eq 0 ]; then
			tst_res TCONF "${unshare_cmd} not supported."
		else
			tst_res TFAIL "${unshare_cmd} failed."
		fi
		return
	fi

	case ${verify_cmd} in
	id*)
		check_id "$(cat temp)" "${exp_result}" "${unshare_cmd}"
		;;
	mount*)
		check_mount "dir_B" "${exp_result}" "${unshare_cmd}"
		;;
	esac
}

do_test()
{
	case $1 in
	1) unshare_test "--user" "id -u" "65534";;
	2) unshare_test "--user" "id -g" "65534";;
	3) unshare_test "--user --map-root-user" "id -u" "0";;
	4) unshare_test "--user --map-root-user" "id -g" "0";;
	5) unshare_test "--mount" "mount --bind dir_A dir_B" "unmounted";;
	6) unshare_test "--mount --propagation shared" \
			"mount --bind dir_A dir_B" "mounted";;
	7) unshare_test "--user --map-root-user --mount" \
			"mount --bind dir_A dir_B" "unmounted";;
	8) unshare_test "--user --map-root-user --mount --propagation shared" \
			"mount --bind dir_A dir_B" "unmounted";;
	esac
}

. tst_test.sh
tst_run
