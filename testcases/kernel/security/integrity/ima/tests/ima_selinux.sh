#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2021 Microsoft Corporation
# Copyright (c) Linux Test Project, 2021-2025
# Author: Lakshmi Ramasubramanian <nramas@linux.microsoft.com>
#
# Verify measurement of SELinux policy hash and state.
# Test requires ima_policy=critical_data kernel command line and example IMA
# policy loadable with LTP_IMA_LOAD_POLICY=1.
#
# Relevant kernel commits:
# * fdd1ffe8a812 ("selinux: include a consumer of the new IMA critical data hook")
# * 2554a48f4437 ("selinux: measure state and policy capabilities")

TST_NEEDS_CMDS="awk cut grep tail"
TST_CNT=2
TST_SETUP="setup"
TST_MIN_KVER="5.12"

REQUIRED_POLICY_CONTENT='selinux.policy'

setup()
{
	SELINUX_DIR=$(tst_get_selinux_dir)
	[ "$SELINUX_DIR" ] || tst_brk TCONF "SELinux is not enabled"

	require_ima_policy_cmdline "critical_data"
}

# Format of the measured SELinux state data.
#
# initialized=1;enforcing=0;checkreqprot=1;
# network_peer_controls=1;open_perms=1;extended_socket_class=1;
# always_check_network=0;cgroup_seclabel=1;nnp_nosuid_transition=1;
# genfs_seclabel_symlinks=0;
validate_policy_capabilities()
{
	local measured_cap measured_value expected_value
	local inx=7

	# Policy capabilities flags start from "network_peer_controls"
	# in the measured SELinux state at offset 7 for 'awk'
	while [ $inx -lt 20 ]; do
		measured_cap=$(echo $1 | awk -F'[=;]' -v inx="$inx" '{print $inx}')
		inx=$(($inx + 1))

		measured_value=$(echo $1 | awk -F'[=;]' -v inx="$inx" '{print $inx}')
		expected_value=$(cat "$SELINUX_DIR/policy_capabilities/$measured_cap")
		if [ "$measured_value" != "$expected_value" ]; then
			tst_res $IMA_FAIL "$measured_cap: expected: $expected_value, got: $digest"
			return
		fi

		inx=$(($inx + 1))
	done

	tst_res TPASS "SELinux state measured correctly"
}

# Trigger measurement of SELinux constructs and verify that
# the measured SELinux policy hash matches the hash of the policy
# loaded in kernel memory for SELinux.
test1()
{
	local policy_digest expected_policy_digest algorithm
	local data_source_name="selinux"
	local pattern="data_sources=[^[:space:]]*$data_source_name"
	local tmp_file="$TST_TMPDIR/selinux_policy_tmp_file.txt"

	tst_res TINFO "verifying SELinux policy hash measurement"

	# Trigger a measurement by changing SELinux state
	tst_update_selinux_state

	# Verify SELinux policy hash is measured and then validate that
	# the measured policy hash matches the hash of the policy currently
	# in kernel memory for SELinux
	line=$(grep -E "selinux-policy-hash" $ASCII_MEASUREMENTS | tail -1)
	if [ -z "$line" ]; then
		tst_res $IMA_FAIL "SELinux policy hash not measured"
		return
	fi

	algorithm=$(echo "$line" | cut -d' ' -f4 | cut -d':' -f1)
	policy_digest=$(echo "$line" | cut -d' ' -f6)

	expected_policy_digest="$(compute_digest $algorithm $SELINUX_DIR/policy)" || \
		tst_brk TCONF "cannot compute digest for $algorithm"

	if [ "$policy_digest" != "$expected_policy_digest" ]; then
		tst_res $IMA_FAIL "Digest mismatch: expected: $expected_policy_digest, got: $policy_digest"
		return
	fi

	tst_res TPASS "SELinux policy hash measured correctly"
}

# Trigger measurement of SELinux constructs and verify that
# the measured SELinux state matches the current SELinux
# configuration.
test2()
{
	local measured_data state_file="$TST_TMPDIR/selinux_state.txt"
	local data_source_name="selinux"
	local pattern="data_sources=[^[:space:]]*$data_source_name"
	local tmp_file="$TST_TMPDIR/selinux_state_tmp_file.txt"
	local digest expected_digest algorithm
	local initialized_value
	local enforced_value expected_enforced_value
	local checkreqprot_value expected_checkreqprot_value

	tst_res TINFO "verifying SELinux state measurement"

	# Trigger a measurement by changing SELinux state
	tst_update_selinux_state

	# Verify SELinux state is measured and then validate the measured
	# state matches that currently set for SELinux
	line=$(grep -E "selinux-state" $ASCII_MEASUREMENTS | tail -1)
	if [ -z "$line" ]; then
		tst_res $IMA_FAIL "SELinux state not measured"
		return
	fi

	digest=$(echo "$line" | cut -d' ' -f4 | cut -d':' -f2)
	algorithm=$(echo "$line" | cut -d' ' -f4 | cut -d':' -f1)

	echo "$line" | cut -d' ' -f6 | tst_hexdump -d > $state_file

	expected_digest="$(compute_digest $algorithm $state_file)" || \
	tst_brk TCONF "cannot compute digest for $algorithm"

	if [ "$digest" != "$expected_digest" ]; then
		tst_res $IMA_FAIL "digest mismatch: expected: $expected_digest, got: $digest"
		return
	fi

	# SELinux state is measured as the following string
	#   initialized=1;enforcing=0;checkreqprot=1;
	# Value of 0 indicates the state is ON, and 1 indicates OFF
	#
	# enforce and checkreqprot measurement can be verified by
	# comparing the value of the file "enforce" and "checkreqprot"
	# respectively in the SELinux directory.
	# "initialized" is an internal state and should be set to 1
	# if enforce and checkreqprot are measured correctly.
	measured_data=$(cat $state_file)
	enforced_value=$(echo $measured_data | awk -F'[=;]' '{print $4}')
	expected_enforced_value=$(cat $SELINUX_DIR/enforce)
	if [ "$expected_enforced_value" != "$enforced_value" ]; then
		tst_res $IMA_FAIL "enforce: expected: $expected_enforced_value, got: $enforced_value"
		return
	fi

	checkreqprot_value=$(echo $measured_data | awk -F'[=;]' '{print $6}')
	expected_checkreqprot_value=$(cat $SELINUX_DIR/checkreqprot)
	if [ "$expected_checkreqprot_value" != "$checkreqprot_value" ]; then
		tst_res $IMA_FAIL "checkreqprot: expected: $expected_checkreqprot_value, got: $checkreqprot_value"
		return
	fi

	initialized_value=$(echo $measured_data | awk -F'[=;]' '{print $2}')
	if [ "$initialized_value" != "1" ]; then
		tst_res $IMA_FAIL "initialized: expected 1, got: $initialized_value"
		return
	fi

	validate_policy_capabilities $measured_data
}

. ima_setup.sh
tst_run
