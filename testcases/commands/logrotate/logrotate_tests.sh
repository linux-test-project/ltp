#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2001                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
################################################################################
#
# File :        logrotate_tests.sh
#
# Description:    Test Basic functionality of logrotate command.
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
#
# Author:        Manoj Iyer, manjo@mail.utexas.edu
#
# History:       Dec 23 2002 - Created - Manoj Iyer.
#                Dec 24 2002 - Added   - Test #2 - Test to run logrotate as a
#                                        cron job.
#                Feb 28 2003 - Fixed   - Modified testcase to use functions.
#
# Function: 	chk_ifexists
#
# Description:  - Check if command required for this test exits.
#
# Input:        - $1 - calling test case.
#               - $2 - command that needs to be checked.
#
# Return:		- zero on success.
# 				- non-zero on failure.
chk_ifexists()
{
	RC=0

	which $2 > $LTPTMP/tst_logrotate.err 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		tst_brkm TBROK NULL "$1: command $2 not found."
	fi
	return $RC
}


# Function: init
#
# Description:  - Check if command required for this test exits.
#               - Create temporary directories required for this test.
#               - Initialize global variables.
#
# Return:		- zero on success.
# 				- non-zero on failure.
init()
{
	# Initialize global variables.
	export RC=0
	export TST_TOTAL=2
	export TCID="logrotate"
	export TST_COUNT=0

	# Inititalize cleanup function.
	trap "cleanup" 0

	# create the temporary directory used by this testcase
	if [ -z $TMP ]
	then
		LTPTMP=/tmp/tst_logrotate.$$
	else
		LTPTMP=$TMP/tst_logrotate.$$
	fi

	mkdir -p $LTPTMP > /dev/null 2>&1 || RC=$?
	if [ $RC -ne 0 ]
	then
		 tst_brkm TBROK "INIT: Unable to create temporary directory"
		 return $RC
	fi

	# check if commands tst_*, logrotate, awk and file exists.
	chk_ifexists INIT tst_resm  || return $RC
	chk_ifexists INIT logrotate || return $RC
	chk_ifexists INIT awk       || return $RC
	chk_ifexists INIT file      || return $RC

	return $RC
}


# Function: 	cleanup
#
# Description:  - remove temporaty files and directories. Stop all jobs stated
#                 by this testcase.
#
# Return:		- zero on success.
# 				- non-zero on failure.
cleanup()
{
	#remove all cronjobs that were installed.
	tst_resm TINFO "CLEAN: removing all cron jobs."
	crontab -r > /dev/null 2>&1

	# remove all the temporary files created by this test.
	tst_resm TINFO "CLEAN: removing $LTPTMP"
	rm -fr $LTPTMP
}


# Function: 	test01
#
# Description:  - Test that logrotate logrotate will rotate the logfile
#                 according to the specifications in the config file.
#               - create a config file that will rotate the /var/log/tst_logfile
#                 file.
#               - use force option to force logrotate to cause the log file to
#                 be rotated.
#               - compress the file after rotation.
#
# Return:		- zero on success.
# 				- non-zero on failure.
test01()
{
	count=0
	files=" "
	filesize=0

	TCID=logrotate01
	TST_COUNT=1

	tst_resm TINFO "Test #1: create a configfile $LTPTMP/var_mesg.config"
	tst_resm TINFO "Test #1: use logrotate -f <config> to force rotation"
	tst_resm TINFO "Test #1: this will rotate the log file according to"
	tst_resm TINFO "Test #1: the specification in the configfile."
	tst_resm TINFO "Test #1: 1. rotate /var/log/tst_logfile file."
	tst_resm TINFO "Test #1: 2. compresses it."

	# Check if syslog group exists
	local group="syslog"
	grep -q $group /etc/group || group="root"

	# create config file.
	cat >$LTPTMP/tst_logrotate.conf <<-EOF
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

	# create a log file in /var/log/
	cat >/var/log/tst_logfile <<-EOF
	#****** Begin Log File ********
	# This is a dummy log file.
	#****** End Log File ********
	EOF

	while [ $count -lt 10 ]
	do
		echo "This a dummy log file used to test logrotate command." >> \
			/var/log/tst_logfile
		 		 count=$(( $count+1 ))
	done

	# remove all old-n-stale logfiles.
	for files in /var/log/tst_logfile.*
	do
		rm -f $files > /dev/null 2>&1
	done

	chmod 644 $LTPTMP/tst_logrotate.conf
	logrotate -fv $LTPTMP/tst_logrotate.conf > $LTPTMP/tst_logrotate.out 2>&1 \
		|| RC=$?
	if [ $RC -eq 0 ]
	then
		# check if config file $LTPTMP/tst_logrotate.conf is read
		# check if  /etc/logrotate.d is included/
		# check if 5 rotations are forced.
        # check if compression is done.
		grep "reading config file $LTPTMP/tst_logrotate.conf" \
			$LTPTMP/tst_logrotate.out   > $LTPTMP/tst_logrotate.err 2>&1 || RC=$?
		grep "forced from command line (5 rotations)" \
			$LTPTMP/tst_logrotate.out   > $LTPTMP/tst_logrotate.err 2>&1 || RC=$?
		egrep "compressing new|log with" \
			$LTPTMP/tst_logrotate.out   > $LTPTMP/tst_logrotate.err 2>&1 || RC=$?
		if [ $RC -ne 0 ]
		then
			tst_res TFAIL $LTPTMP/tst_logrotate.err \
				"Test #1: logrotate command failed. Reason:"
		else
			# Check if compressed log file is created.
			if [ -f /var/log/tst_logfile.1.gz ]
			then
				file /var/log/tst_logfile.1.gz | grep "gzip compressed data" \
					> $LTPTMP/tst_logrotate.out 2>&1 || RC=$?
				if [ $RC -eq 0 ]
				then
					tst_resm TPASS \
						"Test #1: logrotate created a compressed file."
				else
					tst_res TFAIL $LTPTMP/tst_logrotate.out \
						"Test #1: Failed to create a compressed file. Reason:"
				fi
				return $RC
			else
				 tst_res TFAIL  $LTPTMP/tst_logrotate.out \
				  "Test #1: Failed create /var/log/tst_logfile.1.gz. Reason:"
				return $RC
			fi
		fi
	else
		tst_res TFAIL $LTPTMP/tst_logrotate.out \
		"Test #1: logrotate command exited with $RC return code. Output:"
	fi
	return $RC
}


test02()
{
# Test #2
# Test that logrotate logrotate will rotate the logfile if the logfile
# exceeds a certain size.
#     - create a config file that will rotate the /var/log/tst_largelogfile.
#     - run logrotate in a cron job that runs every minute.
#     - add messages to the logfile until it gets rotated when a re-dittermined
#        size is reached.

export TCID=logrotate02
export TST_COUNT=2
RC=0

tst_resm TINFO "Test #2: create a configfile $LTPTMP/tst_largelog.conf"
tst_resm TINFO "Test #2: logrotate $LTPTMP/tst_largelog.conf - cronjob"
tst_resm TINFO "Test #2: set to rotate tst_largelogfile when size > 2K"


# create config file.
cat >$LTPTMP/tst_largelog.conf <<EOF
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

# create the pseudo-log file.
cat >/var/log/tst_largelogfile <<EOF
# This is a psuedo-log file. This file will grow to a 2k size before
# getting rotated.
EOF

# create logrotate cron job.
cat >$LTPTMP/tst_logrotate.cron <<EOF
* * * * * logrotate $LTPTMP/tst_largelog.conf
EOF

chmod 777 $LTPTMP/tst_logrotate.cron > /dev/null 2>&1

tst_resm TINFO "Test #2: Installing cron job to run logrotate"
crontab $LTPTMP/tst_logrotate.cron > $LTPTMP/tst_logrotate.out 2>&1 || RC=$?
if [ $RC -ne 0 ]
then
    echo "Exit status of crontab command: $RC" >> tst_logrotate.out 2>/dev/null
    tst_brk TBROK $LTPTMP/tst_logrotate.out NULL \
        "Test #2: crontab Broke while installing cronjob. Reason:"
    TFAILCNT=$(( $TFAILCN+1 ))
else
    tst_resm TINFO "Test #2: Cronjob installed successfully"
fi

# cron job to increase the log file size.
cat >$LTPTMP/tst_addtolog.cron <<EOF

* * * * * echo "To Err Is Human, To Really Screw Up You Need A Computer."  >>/var/log/tst_largelogfile 2>/dev/null
EOF

tst_resm TINFO "Test #2: Installing cron job to increase logsize"
crontab $LTPTMP/tst_addtolog.cron > $LTPTMP/tst_logrotate.out 2>&1 || RC=$?
if [ $RC -ne 0 ]
then
    echo "Exit status of crontab command: $RC" >> tst_logrotate.out 2>/dev/null
    tst_brk TBROK $LTPTMP/tst_logrotate.out NULL \
        "Test #2: crontab Broke while installing cronjob. Reason:"
    TFAILCNT=$(( $TFAILCN+1 ))
else
    tst_resm TINFO "Test #2: Cronjob installed successfully"
fi

# let cron jobs get started.
sleep 10s

# increase the log file size.

# wait for the /var/log/tst_largelogfile to be filled to a size greater than 2k
tst_resm TINFO "Test #2: Checking if file size is > 2k"
tst_resm TINFO "Test #2: Pls be patient this will take some time."
tst_resm TINFO "Test #2: or killall -9 logrotate02 to skip.."
if [ -f `which awk` ]
then
    while [ $filesize -lt 2046 ]
    do
        filesize=`ls -l /var/log/tst_largelogfile | awk '{print $5}'`
    done
	# wait for 1m  and check if logrotate has rotated the logfile. The cron job
	# that does a logrotate runs every 1 minute so give the cron a minute...
	sleep 1m
else
	tst_resm TINFO "Test #2: No AWK installed ... sleeping for 10mts"
	sleep 10m
fi


if [ -f /var/log/tst_largelogfile.1.gz ]
then
    file /var/log/tst_largelogfile.1.gz | grep "gzip compressed data" \
        > $LTPTMP/tst_logrotate.out 2>&1 || RC=$?
    if [ $RC -eq 0 ]
    then
        tst_resm TPASS \
            "Test #1: logrotate worked as cron, created a compressed file."
    else
        tst_res TFAIL $LTPTMP/tst_logrotate.out \
            "Test #1: Failed to create a compressed file. Reason:"
    fi
else
    tst_res TFAIL  $LTPTMP/tst_logrotate.out \
        "Test #1: Failed to create /var/log/tst_largelogfile.1.gz. Reason:"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi

}

# Function:	main
#
# Description:	- Execute all tests and report results.
#
# Exit:			- zero on success
#               - non-zero on failure.

RC=0
init || exit $?

test01 || RC=$?

exit $RC
