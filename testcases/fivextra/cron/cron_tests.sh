################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2002                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and/or modify      ##
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
# File:            cron_tests.sh
#
# Description:    This testcase tests if crontab <filename> installs the cronjob
# and cron schedules the job correctly. The job is set such that it will run
# forever every minute of the day.
# The cronjob runs a program that will print a string followed by the current
# date and time. Five samples are taken inorder to verify if cron job is run
# every minute. It is not practical to check if it works for the remaining
# fields of the crontab file also.
# 
# Author:        Manoj Iyer manjo@mail.utexas.edu
#
# History:
#     Dec - 19 - 2002 - Created.
#     Dec - 20 - 2002 - Correted Test #3, grep for the filename of cronjob 
#                         after executing crontab -l.
#                       - Fixed bug in #3, test was not installing the cronjob.
#                       - Added more informational messages TINFO.
#                       - Changed permissions to this file to 'x'
#     Jun - 20 - 2003 - Modified. Test #3, RC needs to be reset to zero before
#                       checking grepping for output from crontab -l.
#     Jul - 31 - 2003 - added execute privilege
#     Sept 04 2003    - Major rework
#                     - using the utility.sources functions.
#		08 Jan 2004 - (RR) updated to tc_utils.source


# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source


#
# Function:    tc_local_cleanup
#
# Description: perform clean up, remove all temporary files 
#
tc_local_cleanup()
{

    # removed all cron jobs. This is a test machine so
    # dont care if all cron jobs are removed.
    crontab -r 2>$stderr 1>$stdout

    # delete all mails from crontab
    echo "d*" | mail -u root 2>$stderr 1>$stdout
}


# Function:    test01
#
# Description: - Test if crontab <filename> installs the crontab file and 
#                cron schedules the job correctly.
#              
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test01()
{
    tc_register    "crontab functionality"

    tc_info "Test #1: crontab <filename> installs the crontab file"
    tc_info "Test #1: cron schedules the job listed in crontab file."
    
    # create the cron job. The job is to run the program tst1_cronprg.sh
    # every minute, every hour, every day, every month, any weekday. 
    
    echo "* * * * * $TMP/tst1_cronprg.sh" > $TMP/tst1_cronjob.cron
    
    # Create the program that will be run by the cronjob. 
    # This program will print a
    # string and date time information.
    
    cat <<-EOF > $TMP/tst1_cronprg.sh
    #! /bin/bash
    echo "This is crontab testcase $(date)" > $TMP/tst1_cron.out
    exit 0
	EOF

    chmod +x $TMP/tst1_cronprg.sh
    
    # install the cronjob, crontab <filename> does that. Sleep for 10s and the
    # check the /var/log/messages to see if there is a record of any crontab
    # activity.
    
    tc_info "Test #1: Installing cron job ... " 
    crontab $TMP/tst1_cronjob.cron 2>$stderr 1>$stdout
    tc_fail_if_bad $? "Broke while installing cronjob" || return
    
    tc_info "Cronjob installed successfully"

    tc_info "wait for 10s for crontab to update log files"
    sleep 10s
    
    tail -n 10 /var/log/messages | grep crontab | grep REPLACE 2>$stderr \
    1>$stdout
    tc_fail_if_bad $? "crontab activity not recorded in var/log/messages" || return

    # just wait a random time for the cron to kickoff the cronjob.
    tc_info "wait for 1m for crontab to update log files"
    sleep 1m
    
    # The program executed by the cron job tst1_cronprg.sh will record the date
    # and time in a file tst1_cron.out. Extract the minute recorded by the 
    # program
    # into TS_MIN1 sleep for 1m 10s so that the cron will update this file after
    # 1m, extract TS_MIN2 and check if the minute recorded has advanced by 1. 
    # Take 5 such samples, if any one of the fail, flag a failure. 
    
    LOOP_CNTR=5
    TS_MIN1=0

    # check if tst1_cron.out file was created. 
    [ -f  $TMP/tst1_cron.out ] || tc_fail_if_bad "crontab did not execute" || return
    
    while [ "$LOOP_CNTR" -ne 0 ]
    do
        TS_MIN1=`cat $TMP/tst1_cron.out | cut -f 8 -d " " | cut -f2 -d:`
        if [ "$TS_MIN1" -eq 59 ]
        then
            TS_MIN1=00
        else
            TS_MIN1=$(( $TS_MIN1+1 ))
        fi
            
    
        # wait for the cronjob to update the tst1_cron.out file.
        tc_info "wait for 1m 2s for crontab to update log files"
        sleep 1m 2s
    
        # check the time recorded in the tst1_cron.out file, 
        # this should be 1 minute ahead of what was recored earlier.
    
        TS_MIN2=$(cat $TMP/tst1_cron.out | cut -f 8 -d " " | cut -f2 -d:)
    
        [ "$TS_MIN2" -ne "$TS_MIN1" ] && \
        tc_fail_if_bad $? "Expected $TS_MIN2 \n Received $TS_MIN1" || return

        LOOP_CNTR=$(( $LOOP_CNTR-1 ))
    done
    
    # check if var/log/messages file was updated.
    grep "CMD ($TMP/tst1_cronprg.sh)" /var/log/messages 2>$stderr 1>$stdout
    tc_pass_or_fail $? "var/log/messages file was not updated"
}


# Function:    test02
#
# Description: - Test if crontab -r removes the installed  crontab file 
#              
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test02()
{
    tc_register    "crontab functionality"
    tc_info "Test #2: crontab -r removes the crontab file." 
    
    cat <<-EOF > $TMP/tst2_cronjob.cron
    * * * * * $TMP/tst2_cronprg.sh
	EOF
    
    cat <<-EOF > $TMP/tst2_cronprg.sh
    #! /bin/sh
    
    echo "Hello Hell"
    exit 0
	EOF
    
    chmod +x  $TMP/tst2_cronprg.sh &>/dev/null
    
    tc_info "Test #2: installing crontab file."
    
    crontab $TMP/tst2_cronjob.cron 2>$stderr 1>$stdout 
    tc_fail_if_bad $? "crontab broke while installing cronjob." || return
    
    tc_info "wait for 10s for crontab to update log files"
    sleep 10s
    
    tail -n 10 /var/log/messages | grep crontab | grep REPLACE 2>$stderr 2>&1
    tc_fail_if_bad $? "crontab activity not recorded in var/log/messages." || return
    
    tc_info "Test #2: uninstalling crontab file."
    crontab -r  2>$stderr 1>$stdout
    tc_fail_if_bad $? "broke while uninstalling cron jobs" || return
    
    tail -n 10 /var/log/messages | grep DELETE 2>$stderr 1>$stdout
    tc_pass_or_fail $? "no /var/log/messages entry for removed cron job"
}



# Function:    test02
#
# Description: - Test if crontab -l lists the cronjob installed.
#              
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test03()
{
    tc_register    "crontab functionality"
    tc_info "Test #3: crontab -l lists the cronjobs installed"
    
    cat <<-EOF > $TMP/tst2_cronjob.cron
    * * * * * $TMP/tst2_cronprg.sh
	EOF
    
    cat <<-EOF > $TMP/tst2_cronprg.sh 
    #! /bin/sh
    
    echo "Hello Hell"
    exit 0
	EOF
    
    chmod +x  $TMP/tst2_cronprg.sh &>/dev/null
    
    tc_info "Test #3: installing crontab file ..."
    crontab $TMP/tst2_cronjob.cron 2>$stderr 1>$stdout
    tc_fail_if_bad $? "crontab failed while installing cronjob" || return
    
    crontab -l | grep "$TMP/tst2_cronprg.sh" 2>$stderr 1>$stdout 
    tc_fail_if_bad $? "failed to list installed cron job" || return

    tc_info "Test #3: uninstalling crontab file."
    crontab -r &>/dev/null || RC=$?
    tc_fail_if_bad $? "failed while removing cronjob" || return
    
    crontab -l 2>$stderr 1>$stdout
    [ $? -eq 0 ] && \
    tc_fail_if_bad $? "failed while listing cron jobs" || return

    grep "no crontab for" $TMP/cron_tst2.out 2>$stderr 1>$stdout
    tc_pass_or_fail $? "lists cron jobs that were apparently removed"
}


# Function: main
#
# Description: - call setup function.
#              - execute each test.
#
# Inputs:      NONE
#
# Exit:        zero - success
#              non_zero - failure
#
TST_TOTAL=3
tc_setup
tc_root_or_break || exit
tc_exec_or_break  crontab grep chmod tail sleep date cat || exit

test01 &&\
test02 &&\
test03
