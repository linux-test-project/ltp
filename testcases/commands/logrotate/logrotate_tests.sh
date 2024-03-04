#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) International Business Machines  Corp., 2001
# Author: Manoj Iyer, manjo@mail.utexas.edu
# Description:   Test Basic functionality of logrotate command.
#                Test #1: Test that logrotate -f <file.conf> rotates the logfile
#                as per the specifications in the conf file. Create a file
#                tst_logfile in /var/log/. Create a conf file such that this
#                logfile is set for rotation every week. Execute the command
#                logrotate -f <file.conf>, check to see if it forced rotation.
#                Test #2: Check if logrotate running as a cronjob will rotate a
#                logfile when it exceeds a specific size. Create two cronjobs
#                1. runs a command to log a string to a logfile. 2. runs
#                logrotate <file.conf> every minute. The conf file specifies
#                that the rotation happen only if the log file exceeds 2k file
#                size.


TST_NEEDS_CMDS="crontab file grep logrotate"
TST_TESTFUNC=test
TST_NEEDS_TMPDIR=1
TST_CNT=2
TST_CLEANUP=cleanup

cleanup(){
	(crontab -l | grep -v tst_largelog) | crontab -
	rm -rf /var/log/tst_logfile*
	rm -rf /var/log/tst_largelogfile*
}

test1(){
	# Function:     test01
        #
        # Description:  - Test that logrotate logrotate will rotate the logfile
        #                 according to the specifications in the config file.
        #               - create a config file that will rotate the /var/log/tst_logfile
        #                 file.
        #               - use force option to force logrotate to cause the log file to
        #                 be rotated.
        #               - compress the file after rotation.

	local group="syslog"
	grep -q $group /etc/group || group="root"

	cat >tst_logrotate.conf <<-EOF
        #****** Begin Config file *******
        # create new (empty) log files after rotating old ones
        create

        # compress the log files
        compress

        /var/log/tst_logfile {
                su root $group
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

        for i in `seq 10`
	do
                echo "This a dummy log file used to test logrotate command." >>\
                        /var/log/tst_logfile
        done

	ROD rm -f /var/log/tst_logfile.*
	ROD chmod 644 tst_logrotate.conf
	ROD logrotate -fv tst_logrotate.conf > tst_logrotate.out 2>&1

	EXPECT_PASS grep -q "reading config file tst_logrotate.conf" \
		tst_logrotate.out

	EXPECT_PASS grep -q "forced from command line (5 rotations)" \
		tst_logrotate.out

	EXPECT_PASS grep -E -q "compressing new|log with" tst_logrotate.out

	ROD test -f /var/log/tst_logfile.1.gz

	file /var/log/tst_logfile.1.gz | grep -q "gzip compressed data"
	if [ $? -ne 0 ]; then
		tst_res TFAIL "Test #1: Failed to create a compressed file."
	fi
}

test2(){
        # Test #2
        # Test that logrotate logrotate will rotate the logfile if the logfile
        # exceeds a certain size.
        #     - create a config file that will rotate the /var/log/tst_largelogfile.
        #     - run logrotate in a cron job that runs every minute.
        #     - add messages to the logfile until it gets rotated when a re-dittermined
        #        size is reached.

	cat >tst_largelog.conf <<-EOF
        # create new (empty) log files after rotating old ones
        create
        # compress the log files
        compress
        # RPM packages drop log rotation information into this directory
        include /etc/logrotate.d 
        /var/log/tst_largelogfile {
            rotate 5
            size=2k
        }
	EOF

	ROD chmod 644 tst_largelog.conf

	cat >/var/log/tst_largelogfile <<-EOF
        # This is a psuedo-log file. This file will grow to a 2k size before
        # getting rotated.
	EOF

	local logcontent="To Err Is Human, To Really Screw Up You Need A Computer."
	for i in `seq 40`
	do
		echo "$logcontent" >> /var/log/tst_largelogfile
	done

	local fullcmd=`command -v logrotate`

	# cron job for logrotating
	(crontab -l 2>/dev/null; echo \
		"* * * * * ${fullcmd} $(pwd)/tst_largelog.conf") | crontab -
	if [ $? -ne 0 ]; then
		tst_brk TBROK "create cron job failed."
	fi

	# let cron jobs get started.
	tst_sleep 5s

	# wait for 1m and check if logrotate has rotated the logfile. The cron
	# job that does a logrotate runs every 1 minute so give the cron a 
	# minute...
	tst_sleep 60s

	EXPECT_PASS test -f /var/log/tst_largelogfile.1.gz

	file /var/log/tst_largelogfile.1.gz | grep -q "gzip compressed data"
	if [ $? -ne 0 ]; then
		tst_res TFAIL "Failed to create a compressed file."
	fi

}

. tst_test.sh
tst_run
