#!/bin/bash
################################################################################
#                                                                              #
#  (C) Copyright IBM Corp. 2003                                                #
#                                                                              #
#  This program is free software;  you can redistribute it and/or modify       #
#  it under the terms of the GNU General Public License as published by        #
#  the Free Software Foundation; either version 2 of the License, or           #
#  (at your option) any later version.                                         #
#                                                                              #
#  This program is distributed in the hope that it will be useful, but         #
#  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY  #
#  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    #
#  for more details.                                                           #
#                                                                              #
#  You should have received a copy of the GNU General Public License           #
#  along with this program;  if not, write to the Free Software                #
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA     #
#                                                                              #
################################################################################
#
# File :       sysvinitcmds_tests.sh
#
# Description: This program tests basic functionality of commands in sysvinit
#              package.
#
# Author:      Manoj Iyer  manjo@mail.utexas.edu
#
# History:     July 01 2003 - created - Manoj Iyer
#              July 02 2003 - Modified - Manoj Iyer
#              - removed spurious " character.
#              - Incorporated review comments.
#              - changed redirection from dev/null to a file so that it can 
#                be printed to screen if required.
#              - removed using awk, using cut instead.
#              - using a more generic option with ps command.
#              - removed using multiple tc_pass_or_fail functions, using multiple
#                maybefails instead.
#              Aug 26 2003 - Modified - Manoj Iyer
#              - removed testcases 4 5 6 and 8
#		08 Jan 2004 - (RR) updated to tc_utils.source


# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source


#
# Function:    test01
#
# Description: - Test the functionality of pidof command
#              - Execute a command, and record its pid
#              - Execute pidoff command and compare the pids
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test01()
{
    tc_register    "pidof functionality"
    
    tc_info "Pidof finds the process ids (pids)"
    tc_info "of the named programs and prints those ids"
    tc_info "on standard output."

    # Executing sleep for 30 secs
    tc_info "executing sleep for 30s as a  background"
    killall -9 sleep &>$TCTMP/tst.err

    sleep 30s &
    tc_fail_if_bad $? "failed to sleep for 30s $(cat $TCTMP/tst.err)" \
                 || return 

    # get the pid of sleep command. 
    cmdpid=$(ps -C sleep | grep sleep | awk '{print $1}')

    # get the pid of sleep using pidof command
    cmdpidof=`pidof sleep`
    tc_fail_if_bad $? "commad pidof failed to get the pid of sleep" || return

    # comprare vaules, test fails if they are unequal.
    [ $cmdpid -eq $cmdpidof ]
    tc_pass_or_fail $? "values are not equal."
}


#
# Function:    test02
#
# Description: - Test the functionality of last command
#              - Execute last command and check for the string
#              - "wtemp begins"
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test02()
{
    tc_register    "last functionality"
    
    tc_info "last searches back through the file /var/log/wtmp (or the"
    tc_info "file designated by the -f flag) and displays a list of"
    tc_info "all users logged in (and out) since that file was created."

    # executing last command and checking for string wtemp begins
    tc_info "executing last command and checking for string wtemp begins"

    last | grep "wtmp begins" &>$TCTMP/tst.err
    tc_pass_or_fail $? "failed to find unique sting wtmp begin"
}


#
# Function:    test03
#
# Description: - Test the functionality of lastb command
#              - Execute lastb command and check for unique string
#              - usually this command is disabled so the unique string 
#                "Perhaps this file was removed"
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test03()
{
    tc_register    "lastb functionality"
    
    tc_info "lastb is the same as last, except that by default it"
    tc_info "shows a log of the file /var/log/btmp, which contains"
    tc_info "all the bad login attempts."

    # executing lastb command and checking for unique string
    tc_info "executing lastb command and checking for unique string"

    lastb | grep "Perhaps this file was removed" &>$TCTMP/tst.err
    [ $? -ne 0 ]
    tc_pass_or_fail $? "failed to find unique string $(cat $TCTMP/tst.err)"
}


#
# Function:    test04
#
# Description: - Test the functionality of mesg command
#              - execute mesg command with 'y' as parameter
#              - run mesg command again and verify that permission to write 
#                is set.
#              - repeat the same test for mesg with parameter n
#              - we cannot test by actually writing because we are running as
#                root.
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test04()
{
    tc_register    "mesg functionality"
    
    tc_info "Mesg controls the access to your terminal by others."

    # execute 'mesg y' followed by command 'mesg' with no options and 
    # verify that the access if modified.

    mesg y &>$TCTMP/tst.err
    tc_fail_if_bad $? "failed to set terminal access to yes $(cat $TCTMP/tst.err)" \
                 || return

    mesg 2>&1 | grep "is y" &>$TCTMP/tst.err
    tc_fail_if_bad $? "failed to set terminal access to yes $(cat $TCTMP/tst.err)" \
                 || return

    mesg n &>$TCTMP/tst.err
    tc_fail_if_bad $? "failed to set terminal access to no $(cat $TCTMP/tst.err)" \
                 || return

    mesg 2>&1 | grep "is n" &>$TCTMP/tst.err
    tc_fail_if_bad $? "failed to set terminal access to no $(cat $TCTMP/tst.err)"
}


#
# Function:    test05
#
# Description: - Test the functionality of utempdump command
#              - Execute utempdump command and check for the string
#              - "Utmp dump of /var/run/utmp"
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test05()
{
    tc_register    "utmpdump functionality"
    
    tc_info "utempdump <filename> prints the content of a file "
    tc_info "(usually /var/run/utmp) on standard output in a user "
    tc_info "friendly format."

    # executing utempdump and searching for unique string
    cat <<-EOF >$TCTMP/tst_sysvinit.exp
    Utmp dump of /var/run/utmp
    EOF

    utmpdump /var/run/utmp &>$TCTMP/tst.out
    tc_fail_if_bad $? "utmpdump failed"

    fprep -f $TCTMP/tst_sysvinit.exp $TCTMP/tst.out 
    tc_pass_or_fail $? "did notfind unique string produced by utmpdump"
}


#
# Function:    test06
#
# Description: - Test the functionality of wall command
#              - execute wall command and verify that it broadcasts the content
#                of the test file to all users.
#              - execute wall command and verify that it broadcasts the content
#                of its stdin to all users.
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test06()
{
    tc_register    "wall functionality"
    
    tc_info "Wall sends a message to everybody logged in with"
    tc_info "their mesg permission set to yes."

    # create a file that will be broadcast to all users.
    cat <<-EOF > $TCTMP/tst_walltest.in
    THIS IS A BIG HELLO FROM ROOT
	EOF

    wall $TCTMP/tst_walltest.in &>/devnull
    tc_pass_or_fail $? "wall failed to broadcast file contents to all users" \
        || return

    # wall will broadcast contents of stdin to all users.
    echo "THIS IS A BIG HELLO FROM ROOT" | wall
    tc_pass_or_fail $? "wall failed to broadcast stdin contents to all users"
}


#
# Function:    test07
#
# Description: - Test the functionality of shutdown command
#              - execute shutdown -a and verify that it issues a warning.
#
# Inputs:        NONE
#
# Exit         0 - on success
#              non-zero on failure.
test07()
{
    tc_register    "shutdown functionality"
    
    tc_info "shutdown brings the system down in a secure way."

    shutdown -k now  | tail -n 1 >$TCTMP/tst_shutdcmd.out
    tc_fail_if_bad $? "failed to execute shutdown -k now command" || return

    # create an expected file.
    cat <<-EOF > $TCTMP/tst_shutdcmd.in

    Shutdown cancelled.
	EOF

    diff -iwqB  $TCTMP/tst_shutdcmd.out $TCTMP/tst_shutdcmd.in 2>$stderr
    tc_pass_or_fail $? "did not issue expected warning message."
}


#
# Function:    test08
#
# Description: - Test the functionality of halt command
#              - Execute halt command with '-w' option, this will cause halt
#                to update the /var/log/utmp file and not do the actual halt
#              - use the utmpdump command and verify that an entry was made in
#                /var/log/utmp
#
# Inputs:        NONE
#
# Exit         0 - on success
#              non-zero on failure.
test08()
{
    tc_register    "halt functionality"
    
    tc_info "Halt notes that the system is being brought down in"
    tc_info "the file /var/log/wtmp tells the system to shut down"

    halt -w &>$TCTMP/tst.err
    tc_fail_if_bad $? "failed to execute command halt" || return

    # hopefully in the mean time no one wrote to /var/log/utmp
    utmpdump /var/log/utmp | tail -n 1 | grep shutdown &>$TCTMP/tst.err
    tc_pass_or_fail $? "halt -w failed to update /var/log/utmp"
}


#
# Function:    test09
#
# Description: - Test the functionality of runlevel command
#              - Execute runlevel command, assume that no previous runlevel
#                exists letter N will be printed instead of previous runlevel.
#
# Inputs:        NONE
#
# Exit         0 - on success
#              non-zero on failure.
test09()
{
    tc_register    "runlevel functionality"
    
    tc_info "Runlevel reads the system utmp file to report runlevel"

    runlevel | grep "N" &>$TCTMP/tst.err
    tc_pass_or_fail $? "unexpected previous runlevel, expected 'N' "
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

TST_TOTAL=5
tc_setup        # exits on failure

tc_root_or_break || exit
tc_exec_or_break  awk pidof last lastb mesg utmpdump wall halt poweroff reboot \
          shutdown || exit

test01 &&\
test02 &&\
test03 &&\
#test04 &&\
#test05 &&\
#test06 &&\
test07  &&\
#test08 &&\
test09
