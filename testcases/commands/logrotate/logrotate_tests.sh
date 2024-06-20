#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines Corp., 2001
# Copyright (c) Linux Test Project, 2002-2024
# Author: Manoj Iyer <manjo@mail.utexas.edu>
#
# Test Basic functionality of logrotate command.
#
# Test #1
# Test that logrotate logrotate will rotate the logfile according to the
# specifications in the config file.
# - Create a config file that will rotate the /var/log/tst_logfile file.
# - Use force option to force logrotate to cause the log file to be rotated.
# - Compress the file after rotation.
#
# Test #2
# Test that logrotate logrotate will rotate the logfile if the logfile
# exceeds a certain size.
# - Create a config file that will rotate the /var/log/tst_largelogfile.
# - Run logrotate in a cron job that runs every minute.
# - Add messages to the logfile until it gets rotated when a re-dittermined size
#   is reached.

TST_NEEDS_CMDS="crontab file grep logrotate"
TST_TESTFUNC=test
TST_NEEDS_TMPDIR=1
TST_CNT=2
TST_SETUP=setup
TST_CLEANUP=cleanup

PERMISSION=

setup()
{
	local group="syslog"

	grep -q $group /etc/group || group="root"
	PERMISSION="su root $group"
}

cleanup()
{
	(crontab -l | grep -v tst_largelog) | crontab -
	rm -rf /var/log/tst_logfile*
	rm -rf /var/log/tst_largelogfile*
}

check_log()
{
	local file="$1"

	EXPECT_PASS [ -f "$file" ]

	if ! file "$file" | grep -q "gzip compressed data"; then
		tst_res TFAIL "Failed to create a compressed file"
	fi
}

test1()
{
	cat >tst_logrotate.conf <<-EOF
        #****** Begin Config file *******
        # create new (empty) log files after rotating old ones
        create

        # compress the log files
        compress

        /var/log/tst_logfile {
                $PERMISSION
                rotate 5
                weekly
        }
        #****** End Config file *******
	EOF

	cat >/var/log/tst_logfile <<-EOF
        #****** Begin Log File ********
        # This is a dummy log file.
        #****** End Log File ********
	EOF

	for i in $(seq 10); do
		echo "This a dummy log file used to test logrotate command." >> /var/log/tst_logfile
	done

	ROD rm -f /var/log/tst_logfile.*
	ROD chmod 644 tst_logrotate.conf
	ROD logrotate -fv tst_logrotate.conf > tst_logrotate.out 2>&1

	EXPECT_PASS grep -q "reading config file tst_logrotate.conf" tst_logrotate.out
	EXPECT_PASS grep -q "forced from command line (5 rotations)" tst_logrotate.out
	EXPECT_PASS grep -E -q "compressing new|log with" tst_logrotate.out

	check_log /var/log/tst_logfile.1.gz
}

test2()
{
	cat >tst_largelog.conf <<-EOF
        # create new (empty) log files after rotating old ones
        create
        # compress the log files
        compress
        /var/log/tst_largelogfile {
            $PERMISSION
            rotate 5
            size=2k
        }
	EOF

	ROD chmod 644 tst_largelog.conf

	cat >/var/log/tst_largelogfile <<-EOF
        # This is a psuedo-log file. This file will grow to a 2k size before
        # getting rotated.
	EOF

	for i in $(seq 75); do
		echo "Some text for testing rotation" >> /var/log/tst_largelogfile
	done

	# cron job for logrotating
	(crontab -l 2>/dev/null; echo \
		"* * * * * $(command -v logrotate) $(pwd)/tst_largelog.conf") | crontab -
	if [ $? -ne 0 ]; then
		tst_brk TBROK "Failed to create a cron job"
	fi

	# 5 sec for cron job to start, 1 min for logrotate to rotate the logs
	tst_res TINFO "sleep 1 min to wait for rotating logs"
	tst_sleep 65s

	check_log /var/log/tst_largelogfile.1.gz
}

. tst_test.sh
tst_run
