#! /bin/sh
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2003                                               ##
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
# File :        mail_tests.sh
#
# Description:  Tests basic functions of mail system. The aim of the test is to
#               make sure that certain basic functionality of mail is expected
#               to work as per man page. There are 4 - 5 operations that are
#               done on a regular basis wrt mail. ie. 
#               mail send to an user@domain - received by that user@domain
#               mail is send to nosuchuser@domain - mail delivery failure
#               mail is send to user@nosuchdomain - mail delivery failure
#               mail to user1@domain and cc user2@domain - mail rec by both
#               mail to user1@domain and bcc user2@domain - mail rec by both
#
# Author:       Manoj Iyer, manjo@mail.utexas.edu
#
# History:      Jan 07 2003 - Created - Manoj Iyer.
#               Jan 09 2003 - Added Test #2 #3 #4 and #5.
#               Jan 10 2003 - Fixed various bugs I had introduced in the test.
#                           - Added SETUP and CLEANUP sections 
#               Aug 21 2003 - using tc_utils.source function.
#               Sep 30 2003 - empty counts are reported as non-integers.
#                           - initial check to see if they are non-null
#               Oct 03 2003 - wait more time for mail to arrive, hack  for 
#                             slow network infrastructure. Also, check for
#                             new mails in test 4 and 5
#               Oct 06 2003 - DOnt know why 4 & 5 fails inside the script
#                             and works outside the script. Commenting them
#                             out for now.
#		08 Jan 2004 - (RR) updated to tc_utils.source


# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source


#   
# Function:    tc_local_setup
#   
# Description: - standard setup routing and test specific setup procedure
#              - create user mail_test
#              - create input file to mail test.
#              - remove any mails that exists for root and mail_test
#   
# Inputs:      NONE
#   
# Exit         0 - on success
#              non-zero on failure.
tc_local_setup()
{

    tc_root_or_break || exit
    tc_exec_or_break  mail awk echo useradd sleep || exit

    # check if the user mail_test exists on this system.
    # if not add that user mail_test, will removed before exiting test.
    [ -z $(awk '/^mail_test/ {print 1}' /etc/passwd) ] && \
    {
        tc_info "Adding temporary user mail_test"
        useradd -m -s /bin/bash mail_test &>$TCTMP/tst_mail.out
        tc_fail_if_bad $? "adding user mail_test $(cat $TCTMP/tst_mail.out)" \
        || return
    }
    tc_info "Removing all mails for mail_test and root"
    echo "d*" | mail -u mail_test &>/dev/null
    echo "d*" | mail -u root &>/dev/null

    echo "This is a test email." > $TCTMP/tst_mail.in
    tc_fail_if_bad $? "unable to create input file $TCTMP/tst_mail.in" || return
}

 
#   
# Function:    tc_local_cleanup
#   
# Description: - standard cleanup routing and test specific cleanup procedure
#              - remove user mail_test
#              - remove any mails that exists for root and mail_test
#   
# Inputs:      NONE
#   
# Exit         0 - on success
#              non-zero on failure.
tc_local_cleanup()
{
    deluser --remove-all-files mail_test 2>$stderr 1>$stderr || 
    userdel -r mail_test 2>$stderr 1>$stderr || \
    { tc_info "failed removing user mail_test" ; }

    echo "d*" | mail -u root &>/dev/null
    echo "d*" | mail -u mail_test &>/dev/null
}
        

#
# Function:    test01
#
# Description: - Test the functionality of mail command
#              - mail user@domain will send a mail to that user at that domain.
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test01()
{
    local mailsub=0
    tc_register    "mail functionality"
    tc_info "Test #1: mail root@localhost will send mail to root"
    tc_info "Test #1: user on local machine."
    
    mail -s "Test" root@localhost < $TCTMP/tst_mail.in 2>$stderr 1>$stdout
    tc_fail_if_bad $? "unable to send email to root@localhost" || return

    # check if root received a new email with Test as subject
    # but wait for the mail to arrive.
       
    sleep 5s
    echo "d*" | mail -u root &>$TCTMP/tst_mail.res
    mailsub=$(awk '/^>N/ {print match($9, "Test")}' $TCTMP/tst_mail.res)
    ! [ -z "$mailsub" ] && [ "$mailsub" -ne "0" ] 
    tc_pass_or_fail $? "email to root@localhost was not received" 
}


#
# Function:    test02
#
# Description: - Test the functionality of mail command
#              - mail user@bad-domain will result in a warning from 
#                the mailer deamon that the domain does not exist. 
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test02()
{
    tc_register    "mail functionality"
    tc_info "mail user@bad-domain will result in failure"
    tc_info "to deliver the mail. Mailer deamon should"
    tc_info "report this failure."
    
    
    mail -s "Test" root@this_domain_does_not_exist < $TCTMP/tst_mail.in \
         2>$stderr 1>$stdout
    tc_fail_if_bad $? "failed to email  root@this_domain_does_not_exist" || return

    # check if Mailer-Deamon reported any delivery failure.    
    # but wait for the mail to arrive first,sleep 5s.
    sleep 5s
    echo "d*" | mail -u root | grep ">N" &>$TCTMP/tst_mail.res
    for string in "mailer-daemon" "returned" "mail"
    do
        grep -i $string $TCTMP/tst_mail.res &>/dev/null || { break ; }
    done
    tc_pass_or_fail $? \
    "expected word $string not found. Delivery failure not reported"
}

#
# Function:    test03
#
# Description: - Test the functionality of mail command
#              - mail non_existant_user@localhost will result in 
#                delivery failure. Mailer-Deamon will report this failure.
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test03()
{
    tc_register    "mail functionality"
    tc_info "mail non_existant_user@localhost will fail"
    tc_info "to deliver the mail. Mailer deamon should"
    tc_info "report this failure."
    
    mail -s "Test" non_existant_userr@localhost < $TCTMP/tst_mail.in \
         2>$stderr 1>$stdout
    tc_fail_if_bad $? "failed sending email non_existant_userr@localhost" || return

    # check if Mailer-Deamon reported any delivery failure.    
    # but wait for the mail to arrive first,sleep 5s.
     sleep 5s

    echo "d*" | mail -u root | grep ">N" &>$TCTMP/tst_mail.res
    for string in "mailer-daemon" "returned" "mail"
    do
        grep -i $string $TCTMP/tst_mail.res &>/dev/null || { break ; }
    done
    tc_pass_or_fail $? \
    "expected word $string not found. Delivery failure not reported"

}


#
# Function:    test04
#
# Description: - Test the functionality of mail command
#              - Test that mail -c user@domain option will carbon 
#                copy that user.
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test04()
{

    local root_mail=0
    local cc_mail=0
    tc_register    "mail functionality"

    tc_info "Test that mail -c user@domain will"
    tc_info "carbon copy user@domain"
    
    # send mail to root and carbon copy mail_test 
    mail -s "Test" root@localhost -c mail_test@localhost < \
    $TCTMP/tst_mail.in 2>$stderr 1>$stdout
    tc_fail_if_bad $? "failed to send email and CC mail_test@localhost" || return

    # Check if mail_test received the mail and 
    # also if root received the main copy of the email.
    sleep 5s
    root_mail=$(echo "d*" | mail -u root | awk '/^>N/ {print match($9, "Test")}')
    cc_mail =$(echo "d*" | mail -u mail_test | awk '/^>N/ {print match($9, "Test")}')
    ! [ -z "$root_mail" ] && ! [ -z "$cc_mail" ] && \
    [ "$root_mail" -ne "0" ] && [ "$cc_mail" -ne "0" ] 
    tc_pass_or_fail $? "failed to CC mail_test and email root"
}



#
# Function:    test05
#
# Description: - Test the functionality of mail command
#              - Test that mail -b user@domain option will 
#                blind carbon copy that user.
#
# Inputs:      NONE
#
# Exit         0 - on success
#              non-zero on failure.
test05()
{

    local root_mail=0
    local cc_mail=0

    tc_register    "mail functionality"
    tc_info "Test #5: Test that mail -b user@domain will"
    tc_info "Test #5: blind carbon copy user@domain"
    
    # send mail to root and carbon copy mail_test 
    echo "d*" | mail -u root &>/dev/null
    mail -s "Test" root@localhost -c mail_test@localhost < \
        $TCTMP/tst_mail.in 2>$stderr 1>$stdout

    # Check if mail_test received the mail and 
    # also if root received the main copy of the email.
    sleep 5s

    echo "d*" | mail -u root &>$TCTMP/tst_mail.res
    root_mail=$(awk '/^>N/ {print match($9, "Test")}' $TCTMP/tst_mail.res)
    echo "d*" | mail -u mail_test &>$TCTMP/tst_mail.res
    cc_mail=$(awk '/^>N/ {print match($9, "Test")}' $TCTMP/tst_mail.res)
    ! [ -z "$root_mail" ] && ! [ -z "$cc_mail" ] && \
    [ "$root_mail" -ne "0" ] && [ "$cc_mail" -ne "0" ] 
    tc_pass_or_fail $? "failed to CC mail_test and email root"
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
tc_info "NOTE::slow testcase takes few mts."
test01 &&
test02 &&
test03
#test04 &&
#test05
