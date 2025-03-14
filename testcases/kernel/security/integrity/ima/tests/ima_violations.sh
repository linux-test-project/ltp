#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2009 IBM Corporation
# Copyright (c) 2018-2025 Petr Vorel <pvorel@suse.cz>
# Author: Mimi Zohar <zohar@linux.ibm.com>
#
# Test whether ToMToU and open_writer violations invalidatethe PCR and are logged.

TST_SETUP="setup"
TST_CLEANUP="cleanup"
TST_CNT=8

REQUIRED_BUILTIN_POLICY="tcb"
REQUIRED_POLICY_CONTENT='violations.policy'

setup()
{
	FILE="test.txt"
	IMA_VIOLATIONS="$SECURITYFS/ima/violations"
	LOG="/var/log/messages"
	PRINTK_RATE_LIMIT=

	if status_daemon auditd; then
		LOG="/var/log/audit/audit.log"
	elif tst_check_cmds sysctl; then
		PRINTK_RATE_LIMIT=`sysctl -n kernel.printk_ratelimit`
		sysctl -wq kernel.printk_ratelimit=0
	fi

	if [ ! -e "$LOG" ]; then
		tst_brk TCONF "log file not found, install auditd"
	fi
	tst_res TINFO "using log $LOG"
	exec 3< $LOG || tst_brk TBROK "failed to read log file"
}

cleanup()
{
	[ "$PRINTK_RATE_LIMIT" ] && \
		sysctl -wq kernel.printk_ratelimit=$PRINTK_RATE_LIMIT
}

open_file_read()
{
	exec 3< $FILE || exit 1
}

close_file_read()
{
	exec 3>&-
}

open_file_write()
{
	exec 4> $FILE || exit 1
	echo 'test writing' >&4
}

close_file_write()
{
	exec 4>&-
}

open_file_write2()
{
	exec 5> $FILE || tst_brk TBROK "exec 5> $FILE failed"
	echo 'test writing2' >&5
}

close_file_write2()
{
	exec 5>&-
}

get_count()
{
	local search="$1"
	echo $(grep -c "$search.*$FILE" $LOG)
}

validate()
{
	local num_violations="$1"
	local count="$2"
	local search="$3"
	local expected_violations="$4"
	local max_attempt=3
	local count2 i num_violations_new

	for i in $(seq 1 $max_attempt); do
		read num_violations_new < $IMA_VIOLATIONS
		count2="$(get_count $search)"
		if [ -z "$expected_violations" -a $(($num_violations_new - $num_violations)) -gt 0 ] || \
		   [ $(($num_violations_new - $num_violations)) -eq $expected_violations ]; then
			[ -z "$expected_violations" ] && expected_violations=1
			if [ $count2 -gt $count ]; then
				tst_res TPASS "$expected_violations $search violation(s) added"
				return
			else
				tst_res TINFO "$search not found in $LOG ($i/$max_attempt attempt)..."
				tst_sleep 1s
			fi
		elif [ $(($num_violations_new - $num_violations)) -gt 0 ]; then
			tst_res $IMA_FAIL "$search too many violations added: $num_violations_new - $num_violations"
			return
		else
			tst_res $IMA_FAIL "$search violation not added"
			return
		fi
	done
	tst_res $IMA_FAIL "$search not found in $LOG"
}

test1()
{
	tst_res TINFO "verify open writers violation"

	local search="open_writers"
	local count num_violations

	read num_violations < $IMA_VIOLATIONS
	count="$(get_count $search)"

	open_file_write
	open_file_read
	close_file_read
	close_file_write

	validate $num_violations $count $search
}

test2()
{
	tst_res TINFO "verify ToMToU violation"

	local search="ToMToU"
	local count num_violations

	read num_violations < $IMA_VIOLATIONS
	count="$(get_count $search)"

	open_file_read
	open_file_write
	close_file_write
	close_file_read

	validate $num_violations $count $search
}

test3()
{
	tst_res TINFO "verify open_writers using mmapped files"

	local search="open_writers"
	local count num_violations

	read num_violations < $IMA_VIOLATIONS
	count="$(get_count $search)"

	echo 'testing testing' > $FILE

	ima_mmap -f $FILE &
	# wait for violations appear in logs
	tst_sleep 1s

	open_file_read
	close_file_read

	validate $num_violations $count $search

	# wait for ima_mmap to exit, so we can umount
	tst_sleep 2s
}

test4()
{
	tst_res TINFO "verify limiting single open writer violation"

	if tst_kvcmp -lt 6.14; then
		tst_brk TCONF "Minimizing violations requires kernel 6.14 or newer"
	fi

	local search="open_writers"
	local count num_violations

	read num_violations < $IMA_VIOLATIONS
	count="$(get_count $search)"

	open_file_write
	open_file_read
	close_file_read

	open_file_read
	close_file_read

	close_file_write

	validate "$num_violations" "$count" "$search" 1
}

test5()
{
	tst_res TINFO "verify limiting multiple open writers violations"

	local search="open_writers"
	local count num_violations

	read num_violations < $IMA_VIOLATIONS
	count="$(get_count $search)"

	open_file_write
	open_file_read
	close_file_read

	open_file_write2
	open_file_read
	close_file_read
	close_file_write2

	open_file_read
	close_file_read

	close_file_write

	validate "$num_violations" "$count" "$search" 1
}

test6()
{
	tst_res TINFO "verify new open writer causes additional violation"

	local search="open_writers"
	local count num_violations

	read num_violations < $IMA_VIOLATIONS
	count="$(get_count $search)"

	open_file_write
	open_file_read
	close_file_read

	open_file_read
	close_file_read
	close_file_write

	open_file_write
	open_file_read
	close_file_read
	close_file_write
	validate "$num_violations" "$count" "$search" 2
}

test7()
{
	tst_res TINFO "verify limiting single open reader ToMToU violations"

	local search="ToMToU"
	local count num_violations

	read num_violations < $IMA_VIOLATIONS
	count="$(get_count $search)"

	open_file_read
	open_file_write
	close_file_write

	open_file_write
	close_file_write
	close_file_read

	validate "$num_violations" "$count" "$search" 1
}

test8()
{
	tst_res TINFO "verify new open reader causes additional violation"

	local search="ToMToU"
	local count num_violations

	read num_violations < $IMA_VIOLATIONS
	count="$(get_count $search)"

	open_file_read
	open_file_write
	close_file_write
	close_file_read

	open_file_read
	open_file_write
	close_file_write
	close_file_read

	validate "$num_violations" "$count" "$search" 2
}

. ima_setup.sh
. daemonlib.sh
tst_run
