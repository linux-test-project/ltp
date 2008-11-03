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
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
##                                                                            ##
################################################################################
#
# File:			cron_tests.sh
#
# Description:	This testcase tests if crontab <filename> installs the cronjob
# and cron schedules the job correctly. The job is set such that it will run
# forever every minute of the day.
# The cronjob runs a program that will print a string followed by the current
# date and time. Five samples are taken inorder to verify if cron job is run
# every minute. It is not practical to check if it works for the remaining
# fields of the crontab file also.
# 
# Author:		Manoj Iyer manjo@mail.utexas.edu
#
# History:
# 	Dec - 19 - 2002 - Created.
#	Dec - 20 - 2002 - Correted Test #3, grep for the filename of cronjob 
#                         after executing crontab -l.
#                       - Fixed bug in #3, test was not installing the cronjob.
#                       - Added more informational messages TINFO.
#                       - Changed permissions to this file to 'x'

export TST_TOTAL=3

if [ -z "$LTPTMP" -a -z "$TMPBASE" ]
then
    LTPTMP=/tmp
else
    LTPTMP=$TMPBASE
fi

if [ -z "$LTPBIN" -a -z "$LTPROOT" ]
then
    LTPBIN=./
else
    LTPBIN=$LTPROOT/testcases/bin
fi

# Set return code RC variable to 0, it will be set with a non-zero return code
# in case of error. Set TFAILCNT to 0, increment if there occures a failure.

LOCTMP=${PWD}/tmp
TFAILCNT=0
RC=0

# Test #1
# Test if crontab <filename> installs the crontab file and cron schedules the 
# job correctly.

export TCID=cron01
export TST_COUNT=1

$LTPBIN/tst_resm TINFO "Test #1: crontab <filename> installs the crontab file"
$LTPBIN/tst_resm TINFO "Test #1: cron schedules the job listed in crontab file."

# create the cron job. The job is to run the program tst1_cronprg.sh
# every minute, every hour, every day, every month, any weekday. 

cat > $LTPTMP/tst1_cronjob.cron <<EOF 
* * * * * $LTPTMP/tst1_cronprg.sh
EOF

# Create the program that will be run by the cronjob. This program will print a
# "Hello Hell" string and date time information.

cat > $LTPTMP/tst1_cronprg.sh <<EOF
#! /bin/sh

DATE=\`LANG= date\`
echo "Hello Hell today is \$DATE " > $LTPTMP/tst1_cron.out 2>&1
exit 0
EOF

chmod +x $LTPTMP/tst1_cronprg.sh

# install the cronjob, crontab <filename> does that. Sleep for 10s and the
# check the /var/log/messages to see if there is a record of any crontab
# activity.

$LTPBIN/tst_resm TINFO "Test #1: Installing cron job ... " 
crontab $LTPTMP/tst1_cronjob.cron >$LTPTMP/cron_tst2n1.out 2>&1
RC=$?

if [ $RC -ne 0 ]
then
	$LTPBIN/tst_brk TBROK $LTPTMP/cron_tst2n1.out NULL \
		"Test #1: crontab Broke while installing cronjob. Reason:"
		 TFAILCNT=$(( $TFAILCNT+1 ))
else
	$LTPBIN/tst_resm TINFO "Test #1: Cronjob installed successfully"
fi

sleep 10s

tail -n 10 /var/log/messages | grep crontab | grep REPLACE \
	> $LTPTMP/cron_tst2n1.out 2>&1
RC=$?
#####
# Some implementations log cron info to /var/log/cron instead...
#####
if [ "$RC" -ne 0 -a -f /var/log/cron ]; then
	$LTPBIN/tst_resm TINFO "Test #1: /var/log/cron: Trying altenate log..."
	tail -n 10 /var/log/cron | grep crontab | grep REPLACE \
	    > $LTPTMP/cron_tst2n1.out 2>&1
	RC=$?
fi
if [ $RC -ne 0 ]
then
	$LTPBIN/tst_resm TFAIL \
		"Test #1: crontab activity not recorded in /var/log/messages."
		 TFAILCNT=$(( $TFAILCNT+1 ))
else
	$LTPBIN/tst_resm TINFO \
		"Test #1: cron activity logged in /var/log/messages"
fi

# just wait a random time for the cron to kickoff the cronjob.
#####
# Sleep enough to get _just past_ the start of the next minute --
# like 2 or 3 seconds past... since the loop below sleeps for 62
# seconds, we should start this 5-iteration loop closely following
# the start of a minute...
#####
sleep 1m	# allows cron to run once
XS=$(expr 60 - $(date | awk '{print $4}' | cut -f3 -d:))
[ "$XS" -ne 0 ] && sleep ${XS}s		# sleep to the _next_ minute
sleep 3					# ... for good measure...

# The program executed by the cron job tst1_cronprg.sh will record the date
# and time in a file tst1_cron.out. Extract the minute recorded by the program
# into TS_MIN1 sleep for 1m 10s so that the cron will update this file after
# 1m, extract TS_MIN2 and check if the minute recorded has advanced by 1. Take
# 5 such samples, if any one of the fail, flag a failure. 

LOOP_CNTR=5
TS_MIN1=0
FAILCNT=0

while [ $LOOP_CNTR -ne 0 ]
do
	TS_MIN1=$(awk '{print $8}' $LTPTMP/tst1_cron.out |
	    awk -F: '{printf("%d", $2);}')

	# wait for the cronjob to update the tst1_cron.out file.
	sleep 1m 2s

	# check the time recorded in the tst1_cron.out file, 
        # this should be 1 minute ahead of what was recored earlier.

	TS_MIN2=$(awk '{print $8}' $LTPTMP/tst1_cron.out |
	    awk -F: '{printf("%d", $2);}')

	if [ "x${TS_MIN1}" = "x" ] || [ "x${TS_MIN2}" = "x" ]
	then
		$LTPBIN/tst_resm TFAIL \
			"Test #1: Problem with $LTPTMP/tst1_cron.out file "
		$LTPBIN/tst_resm TFAIL \
			"Test #1: Cause: TS_MIN1= $TS_MIN1; TS_MIN2= $TS_MIN2"
		FAILCNT=$(( $FAILCNT+1 ))
		break;
	fi

	if [ $TS_MIN1 -eq 59 ]
	then
		TS_MIN1=0
	else
		TS_MIN1=$(( $TS_MIN1+1 ))
	fi

	if [ $TS_MIN2 -ne $TS_MIN1 ]
	then
		# if the value of the minute field did not advance by 1
		# flag as failure.
		FAILCNT=$(( $FAILCNT+1 ))
		echo "    Expected $TS_MIN1;     Received $TS_MIN2" \
			> $LTPTMP/tst1_cron.log
		$LTPBIN/tst_res TFAIL $LTPTMP/tst1_cron.log \
			"Test #1: Failed to update every minute. Reason:"
		crontab -r >/dev/null 2>&1
		break
	else
		echo "    Expected $TS_MIN1;     Received $TS_MIN2" \
			> $LTPTMP/tst1_cron.log
		$LTPBIN/tst_res TINFO $LTPTMP/tst1_cron.log \
			"Test #1: Values are good: "
	fi
	LOOP_CNTR=$(( $LOOP_CNTR-1 ))
done

if [ $FAILCNT -eq 0 ]
then
	# check if var/log/messages file was updated.
	grep "CMD ($LTPTMP/tst1_cronprg.sh)" /var/log/messages >$LTPTMP/cron_tst2n1.out 2>&1
	RC=$?
#####
# Some implementations log cron info to /var/log/cron instead...
#####
	if [ "$RC" -ne 0 -a -f /var/log/cron ]; then
		$LTPBIN/tst_resm TINFO "Test #1: /var/log/cron: alternate..."
		grep "CMD ($LTPTMP/tst1_cronprg.sh)" /var/log/cron \
		    >$LTPTMP/cron_tst2n1.out 2>&1
		RC=$?
	fi
	if [ $RC -eq 0 ]
	then
		$LTPBIN/tst_resm TPASS  \
			"Test #1: installed cronjob, and cron executed the cronjob."
	else
		$LTPBIN/tst_res TFAIL $LTPTMP/cron_tst2n1.out \
			"Test #1: Test failed. Reason:"
		 		 TFAILCNT=$(( $TFAILCNT+1 ))
	fi
else
	$LTPBIN/tst_res TFAIL $LTPTMP/cron_tst1.out \
		"Test #1: Cron did not execute every minute"
		 TFAILCNT=$(( $TFAILCNT+1 ))
fi

#remove the cron job that was installed.
crontab -r >/dev/null 2>&1


# Test #2
# Test if crontab -r removes the installed  crontab file 

export TCID=cron02
export TST_COUNT=2

$LTPBIN/tst_resm TINFO "Test #2: crontab -r removes the crontab file." 

cat > $LTPTMP/tst2_cronjob.cron <<EOF
* * * * * $LTPTMP/tst2_cronprg.sh
EOF

cat > $LTPTMP/tst2_cronprg.sh <<EOF
#! /bin/sh

echo "Hello Hell"
exit 0
EOF

chmod +x  $LTPTMP/tst2_cronprg.sh >/dev/null 2>&1

$LTPBIN/tst_resm TINFO "Test #2: installing crontab file."

crontab $LTPTMP/tst2_cronjob.cron >$LTPTMP/cron_tst2n1.out 2>&1

if [ $? -ne 0 ]
then
    $LTPBIN/tst_brk TBROK $LTPTMP/cron_tst2n1.out NULL \
        "Test #2: crontab Broke while installing cronjob. Reason:"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi

sleep 10s

tail -n 10 /var/log/messages | grep crontab | grep REPLACE \
    >$LTPTMP/cron_tst2n1.out 2>&1
RC=$?
#####
# Some implementations log cron info to /var/log/cron instead...
#####
if [ "$RC" -ne 0 -a -f /var/log/cron ]; then
	$LTPBIN/tst_resm TINFO "Test #1: /var/log/cron: alternate..."
	tail -n 10 /var/log/cron | grep crontab | grep REPLACE \
	    >$LTPTMP/cron_tst2n1.out 2>&1
	RC=$?
fi
if [ $RC -ne 0 ]
then
    $LTPBIN/tst_resm TFAIL \
        "Test #2: crontab activity not recorded in var/log/messages."
    TFAILCNT=$(( $TFAILCNT+1 ))
fi

$LTPBIN/tst_resm TINFO "Test #2: uninstalling crontab file."

crontab -r  >$LTPTMP/cron_tst2n1.out 2>&1
RC=$?

if [ $RC -ne 0 ]
then
    $LTPBIN/tst_brk TBROK $LTPTMP/cron_tst2n1.out NULL \
        "Test #2: crontab Broke while installing cronjob. Reason:"
    TFAILCNT=$(( $TFAILCNT+1 ))
else
	tail -n 10 /var/log/messages | grep DELETE >$LTPTMP/cron_tst2n1.out 2>&1
	RC=$?
#####
# Some implementations log cron info to /var/log/cron instead...
#####
	if [ "$RC" -ne 0 -a -f /var/log/cron ]; then
		$LTPBIN/tst_resm TINFO "Test #1: /var/log/cron: alternate..."
		tail -n 10 /var/log/cron | grep DELETE \
		    >$LTPTMP/cron_tst2n1.out 2>&1
		RC=$?
	fi
	if [ $RC -ne 0 ]
	then
		$LTPBIN/tst_resm TFAIL \
			"Test #2: crontab activity not recorded in var/log/messages."
		 		 TFAILCNT=$(( $TFAILCNT+1 ))
	else
		$LTPBIN/tst_resm TPASS "Test #2: crontab removed the cronjob"
	fi
fi


# Test #3
# Test if crontab -l lists the cronjob installed.

export TCID=cron03
export TST_COUNT=3

$LTPBIN/tst_resm TINFO "Test #3: crontab -l lists the cronjobs installed"

cat > $LTPTMP/tst2_cronjob.cron <<EOF
* * * * * $LTPTMP/tst2_cronprg.sh
EOF

cat > $LTPTMP/tst2_cronprg.sh <<EOF
#! /bin/sh

echo "Hello Hell"
exit 0
EOF

chmod +x  $LTPTMP/tst2_cronprg.sh >/dev/null 2>&1

$LTPBIN/tst_resm TINFO "Test #3: installing crontab file ..."
crontab $LTPTMP/tst2_cronjob.cron >$LTPTMP/cron_tst2n1.out 2>&1
if [ $? -ne 0 ]
then
    $LTPBIN/tst_brkm TBROK NULL \
		"Test #3: crontab failed while installing cronjob"
    TFAILCNT=$(( $TFAILCNT+1 ))
else
    $LTPBIN/tst_resm TINFO "Test #3: Cron job installed."
fi

crontab -l | grep "$LTPTMP/tst2_cronprg.sh" >$LTPTMP/cron_tst2n1.out 2>&1
RC=$?
if [ $RC -ne 0 ]
then	
	$LTPBIN/tst_brkm TBROK NULL \
		"Test #3: crontab failed while listing cronjobs installed"
		 TFAILCNT=$(( $TFAILCNT+1 ))
else
	$LTPBIN/tst_resm TINFO \
		"Test #3: crontab -l listed cronjob tst2_cronprg.sh"
fi

$LTPBIN/tst_resm TINFO "Test #3: uninstalling crontab file."
crontab -r >/dev/null 2>&1

if [ $? -ne 0 ]
then	
	$LTPBIN/tst_brkm TBROK NULL "Test #3: crontab failed while removing cronjob"
		 TFAILCNT=$(( $TFAILCNT+1 ))
fi

crontab -l >$LTPTMP/cron_tst2.out 2>&1
if [ $? -ne 0 ]
then	
	grep "no crontab for" $LTPTMP/cron_tst2.out >$LTPTMP/cron_tst2n1.out 2>&1
	RC=$?
	if [ $RC -ne 0 ]
	then
		$LTPBIN/tst_res TFAIL $LTPTMP/cron_tst2n1.out \
			"Test #3: crontab failed removing cronjob. Reason:"
		TFAILCNT=$(( $TFAILCNT+1 ))
	else
		$LTPBIN/tst_resm TINFO "crontab uninstalled all jobs for user"
		$LTPBIN/tst_resm TPASS "crontab did not list any cronjobs"
	fi
else
	$LTPBIN/tst_res TFAIL $LTPTMP/cron_tst2n1.out \
		"Test #3: crontab failed removing cronjob. Reason:"
	TFAILCNT=$(( $TFAILCNT+1 ))
fi

exit $TFAILCNT
