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
# File :		logrotate_tests.sh
#
# Description:	Test Basic functionality of logrotate command.
#
# Author:		Manoj Iyer, manjo@mail.utexas.edu
#
# History:		Dec 23 2002 - Created - Manoj Iyer.
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

# Test #1
# Test that logrotate logrotate will rotate the logfile according to the 
# configfile specification.
#     - create a config file that will rotate the /var/log/messages file.
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


exit $TFAILCNT
