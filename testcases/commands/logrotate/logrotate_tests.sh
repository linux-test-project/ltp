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
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
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
# History:        Dec 23 2002 - Created - Manoj Iyer.
#                Dec 24 2002 - Added   - Test #2 - Test to run logrotate as a
#                                        cron job.
#
#! /bin/sh


export TST_TOTAL=3

if [[ -z $LTPTMP && -z $TMPBASE ]]
then
    LTPTMP=/tmp
else
    LTPTMP=$TMPBASE
fi

if [[ -z $LTPBIN && -z $LTPROOT ]]
then
    LTPBIN=./
else
    LTPBIN=$LTPROOT/testcases/bin
fi

# Set return code RC variable to 0, it will be set with a non-zero return code
# in case of error. Set TFAILCNT to 0, increment if there occures a failure.

TFAILCNT=0
RC=0
RC1=0
RC2=0
count=0
files=" "
filesize=0

# Test #1
# Test that logrotate logrotate will rotate the logfile according to the 
# configfile specification.
#     - create a config file that will rotate the /var/log/tst_logfile file.
#     - use force option to force logrotate to cause the log file to be rotated.
#     - compress the file after rotation.

export TCID=logrotate01
export TST_COUNT=1

$LTPBIN/tst_resm TINFO "Test #1: create a configfile $LTPTMP/var_mesg.config"
$LTPBIN/tst_resm TINFO "Test #1: use logrotate -f <config> to force rotation"
$LTPBIN/tst_resm TINFO "Test #1: this will rotate the log file according to"
$LTPBIN/tst_resm TINFO "Test #1: the specification in the configfile."
$LTPBIN/tst_resm TINFO "Test #1: 1. rotate /var/log/tst_logfile file."
$LTPBIN/tst_resm TINFO "Test #1: 2. compresses it."

# create config file.
cat >$LTPTMP/tst_msg.conf <<EOF
# create new (empty) log files after rotating old ones
create

# compress the log files
compress

# RPM packages drop log rotation information into this directory
include /etc/logrotate.d

/var/log/tst_logfile {
    rotate 5
    weekly
}
EOF

# create a log file in /var/log/
cat >/var/log/tst_logfile <<EOF
# This is a dummy log file.
EOF

while [ $count -lt 10 ]
do
    echo "This a dummy log file used to test logrotate command." >> \
        /var/log/tst_logfile 
    count=$((count+1))
done

# remove all old-n-stale logfiles.
for files in /var/log/tst_logfile.*
do
    rm -f $files &>/dev/null
done

logrotate -f $LTPTMP/tst_msg.conf &>$LTPTMP/tst_sterrout.log || RC=$?

echo "Exit Status = $RC" >>$LTPTMP/tst_sterrout.log 2>/dev/null

if [ $RC -eq 0 ]
then
    grep "done" $LTPTMP/tst_sterrout.log &>/dev/null || RC=$?
 
    if [ $RC -ne 0 ]
    then
        TFAILCNT=$((TFAILCNT+1))
        $LTPBIN/tst_resm TFAIL \
            "Test #1: logrotate command failed while rotating logfiles."
    else
        if [ -f /var/log/tst_logfile.1.gz ]
        then
            file /var/log/tst_logfile.1.gz | grep "gzip compressed data" \
                &>$LTPTMP/tst_sterrout.log || RC=$?
            if [ $RC -eq 0 ]
            then
                $LTPBIN/tst_resm TPASS \
                    "Test #1: logrotate created a compressed file."
            else
                $LTPBIN/tst_res TFAIL $LTPTMP/tst_sterrout.log \
                    "Test #1: Failed to create a compressed file. Reason:"
            fi
        else
             $LTPBIN/tst_res TFAIL  $LTPTMP/tst_sterrout.log \
                "Test #1: Failed to create /var/log/tst_logfile.1.gz. Reason:"
            TFAILCNT=$((TFAILCNT+1))
        fi
    fi
else
    $LTPBIN/tst_res TFAIL $LTPTMP/tst_sterrout.log \
    "Test #1: logrotate command failed rotating logfiles. Reason:"
    TFAILCNT=$((TFAILCNT+1))
fi


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

$LTPBIN/tst_resm TINFO "Test #2: create a configfile $LTPTMP/tst_largelog.conf"
$LTPBIN/tst_resm TINFO "Test #2: logrotate $LTPTMP/tst_largelog.conf - cronjob"
$LTPBIN/tst_resm TINFO "Test #2: set to rotate tst_largelogfile when size > 2K"


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

chmod 777 $LTPTMP/tst_logrotate.cron &>/devnull

$LTPBIN/tst_resm TINFO "Test #2: Installing cron job to run logrotate"
crontab $LTPTMP/tst_logrotate.cron &>$LTPTMP/tst_sterrout.log || RC=$?
if [ $RC -ne 0 ]
then
    echo "Exit status of crontab command: $RC" >> tst_sterrout.log 2>/dev/null
    $LTPBIN/tst_brk TBROK $LTPTMP/tst_sterrout.log NULL \
        "Test #2: crontab Broke while installing cronjob. Reason:"
    TFAILCNT=$((TFAILCN+1))
else
    $LTPBIN/tst_resm TINFO "Test #2: Cronjob installed successfully"
fi

# cron job to increase the log file size.
cat >$LTPTMP/tst_addtolog.cron <<EOF

* * * * * echo "To Err Is Human, To Really Screw Up You Need A Computer."  >>/var/log/tst_largelogfile 2>/dev/null 
EOF

$LTPBIN/tst_resm TINFO "Test #2: Installing cron job to increase logsize"
crontab $LTPTMP/tst_addtolog.cron &>$LTPTMP/tst_sterrout.log || RC=$?
if [ $RC -ne 0 ]
then
    echo "Exit status of crontab command: $RC" >> tst_sterrout.log 2>/dev/null
    $LTPBIN/tst_brk TBROK $LTPTMP/tst_sterrout.log NULL \
        "Test #2: crontab Broke while installing cronjob. Reason:"
    TFAILCNT=$((TFAILCN+1))
else
    $LTPBIN/tst_resm TINFO "Test #2: Cronjob installed successfully"
fi

# let cron jobs get started.
sleep 10s

# increase the log file size.

# wait for the /var/log/tst_largelogfile to be filled to a size greater than 2k
$LTPBIN/tst_resm TINFO "Test #2: Checking if file size is > 2k ..."
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
	$LTPBIN/tst_resm TINFO "Test #2: No AWK installed ... sleeping for 10mts"
	sleep 10m
fi


if [ -f /var/log/tst_largelogfile.1.gz ]
then
    file /var/log/tst_largelogfile.1.gz | grep "gzip compressed data" \
        &>$LTPTMP/tst_sterrout.log || RC=$?
    if [ $RC -eq 0 ]
    then
        $LTPBIN/tst_resm TPASS \
            "Test #1: logrotate worked as cron, created a compressed file."
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/tst_sterrout.log \
            "Test #1: Failed to create a compressed file. Reason:"
    fi
else
    $LTPBIN/tst_res TFAIL  $LTPTMP/tst_sterrout.log \
        "Test #1: Failed to create /var/log/tst_largelogfile.1.gz. Reason:"
    TFAILCNT=$((TFAILCNT+1))
fi

#CLEANUP & EXIT
#remove all cronjobs that were installed.
crontab -r &>/dev/null
# remove all the temporary files created by this test.
#rm -f $LTPTMP/tst_addtolog.cron $LTPTMP/tst_logrotate.cron \
$LTPTMP/tst_largelog.conf $LTPTMP/tst_sterrout.log /var/log/tst_largelogfile* \
$LTPTMP/tst_logfile*

exit $TFAILCNT
