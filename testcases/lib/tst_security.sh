#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2018 Petr Vorel <pvorel@suse.cz>

if [ -z "$TST_LIB_LOADED" ]; then
	echo "please load tst_test.sh first" >&2
	exit 1
fi

[ -n "$TST_SECURITY_LOADED" ] && return 0
TST_SECURITY_LOADED=1

_tst_check_security_modules()
{
	local cmd
	local profiles

	if tst_apparmor_enabled; then
		tst_res TINFO "AppArmor enabled, this may affect test results"
		[ "$TST_DISABLE_APPARMOR" = 1 ] || \
			tst_res TINFO "it can be disabled with TST_DISABLE_APPARMOR=1 (requires super/root)"
		profiles=
		for cmd in $TST_NEEDS_CMDS; do
			tst_apparmor_used_profile $cmd && profiles="$cmd $profiles"
		done
		[ -z "$profiles" ] && profiles="none"
		tst_res TINFO "loaded AppArmor profiles: $profiles"
	fi

	if tst_selinux_enforced; then
		tst_res TINFO "SELinux enabled in enforcing mode, this may affect test results"

		[ "$TST_DISABLE_SELINUX" = 1 ] || \
			tst_res TINFO "it can be disabled with TST_DISABLE_SELINUX=1 (requires super/root)"
		profiles=
		for cmd in $TST_NEEDS_CMDS; do
			tst_selinux_used_profile $cmd && profiles="$cmd $profiles"
		done
		[ -z "$profiles" ] && profiles="none"
		tst_res TINFO "loaded SELinux profiles: $profiles"
	fi
}

# Detect whether AppArmor profiles are loaded
# Return 0: profiles loaded, 1: none profile loaded or AppArmor disabled
tst_apparmor_enabled()
{
	local f="/sys/module/apparmor/parameters/enabled"
	[ -f "$f" ] && [ "$(cat $f)" = "Y" ]
}

# Detect whether AppArmor profile for command is enforced
# tst_apparmor_used_profile CMD
# Return 0: loaded profile for CMD
# Return 1: no profile CMD
tst_apparmor_used_profile()
{
	[ $# -eq 1 ] || tst_brk TCONF "usage tst_apparmor_used_profile CMD"
	local cmd="$1"
	grep -q "$cmd .*(enforce)" /sys/kernel/security/apparmor/profiles 2>/dev/null
}

# Detect whether SELinux is enabled in enforcing mode
# Return 0: enabled in enforcing mode
# Return 1: enabled in permissive mode or disabled
tst_selinux_enforced()
{
	local f="$(tst_get_enforce)"

	[ -f "$f" ] && [ "$(cat $f)" = "1" ]
}

# Detect whether SELinux profile for command is enforced
# tst_selinux_used_profile CMD
# Return 0: loaded profile for CMD
# Return 1: profile for CMD not loaded or seinfo not available
tst_selinux_used_profile()
{
	[ $# -eq 1 ] || tst_brk TCONF "usage tst_selinux_used_profile CMD"
	local cmd="$1"

	if ! tst_cmd_available seinfo; then
		if [ -z "$seinfo_warn_printed" ]; then
			tst_res TINFO "install seinfo to find used SELinux profiles"
			export seinfo_warn_printed=1
		fi
		return 1
	fi
	seinfo -t 2>/dev/null | grep -q $cmd
}

# Try disable AppArmor
# Return 0: AppArmor disabled
# Return > 0: failed to disable AppArmor
tst_disable_apparmor()
{
	tst_res TINFO "trying to disable AppArmor (requires super/root)"
	tst_require_root

	local f="aa-teardown"
	local action

	tst_cmd_available $f && { $f >/dev/null; return; }
	f="/etc/init.d/apparmor"
	if [ -f "$f" ]; then
		for action in teardown kill stop; do
			$f $action >/dev/null 2>&1 && return
		done
	fi
}

# Try disable SELinux
# Return 0: SELinux disabled
# Return > 0: failed to disable SELinux
tst_disable_selinux()
{
	tst_res TINFO "trying to disable SELinux (requires super/root)"
	tst_require_root

	local f="$(tst_get_enforce)"

	[ -f "$f" ] && cat 0 > $f
}

# Get SELinux directory path
tst_get_selinux_dir()
{
	local dir="/sys/fs/selinux"

	[ -f "$dir/enforce" ] && echo "$dir"
}

# Get SELinux enforce file path
tst_get_enforce()
{
	local dir=$(tst_get_selinux_dir)
	[ -z "$dir" ] && return

	local f="$dir/enforce"
	[ -f "$f" ] && echo "$f"
}

tst_update_selinux_state()
{
	local cur_val new_val
	local dir=$(tst_get_selinux_dir)
	[ -z "$dir" ] || return 1

	cur_val=$(cat $dir/checkreqprot)
	[ $cur_val = 1 ] && new_val=0 || new_val=1
	echo $new_val > $dir/checkreqprot
}
