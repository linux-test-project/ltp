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
#                Jan 10 2002 - Fixed various bugs I had introduced in the test.
#                           - Added SETUP and CLEANUP sections 
#

export TST_TOTAL=5

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

RC=0
export TCID=SETUP
export TST_COUNT=1

MAIL_NOT_INSTALLED=
type mail > /dev/null 2>&1
if [ $? != 0 ]; then
    MAIL_NOT_INSTALLED=1
fi

# check if the user mail_test exists on this system.
# if not add that user mail_test, will removed before exiting test.
RC=$(awk '/^mail_test/ {print 1}' /etc/passwd)
if [ -z $RC ]
then
    RC=0
    $LTPBIN/tst_resm TINFO "INIT: Adding temporary user mail_test"
    useradd -m -s /bin/bash mail_test > $LTPTMP/tst_mail.out 2>&1 || RC=$?
    if [ $RC -ne 0 ]
    then
        $LTPBIN/tst_brk TBROK $LTPTMP/tst_mail.out NULL \
            "Test INIT: Failed adding user mail_test. Reason:"
        exit 1
    fi
fi
$LTPBIN/tst_resm TINFO "INIT: Removing all mails for mail_test and root"
echo "d*" | mail -u mail_test > /dev/null 2>&1
echo "d*" | mail -u root > /dev/null 2>&1

# Set return code RC variable to 0, it will be set with a non-zero return code
# in case of error. Set TFAILCNT to 0, increment if there occures a failure.


TFAILCNT=0
RC=0
RC1=0
RC2=0

# Test #1
# Test that mail user@domain will send a mail to that user at that domain.

export TCID=mail01
export TST_COUNT=1

$LTPBIN/tst_resm TINFO "Test #1: mail root@localhost will send mail to root"
$LTPBIN/tst_resm TINFO "Test #1: user on local machine."


if [ -z "$MAIL_NOT_INSTALLED" ]; then
cat > $LTPTMP/tst_mail.in <<EOF
This is a test email.
EOF

mail -s "Test" root@localhost < $LTPTMP/tst_mail.in \
    > $LTPTMP/tst_mail.out 2>&1 || RC=$?
if [ $RC -ne 0 ]
then
    $LTPBIN/tst_res TFAIL $LTPTMP/tst_mail.out \
        "Test #1: mail command failed. Reason: "
    TFAILCNT=$(( $TFAILCNT+1 ))
else
    # check if root received a new email with Test as subject
    # but wait for the mail to arrive.
    
    sleep 10s
    echo "d" | mail -u root > $LTPTMP/tst_mail.res 2>&1
    mailsub=$(awk '/^>N/ {print match($9, "Test")}' $LTPTMP/tst_mail.res)
    if [ $mailsub -ne 0 ]
    then
        $LTPBIN/tst_resm TPASS \
            "Test #1: Mail was send to root & was received"
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/tst_mail.res \
            "Test #1: Mail send to root, but was not received"
        TFAILCNT=$(( $TFAILCNT+1 ))
    fi
fi
else
    $LTPBIN/tst_resm TCONF "mail command not installed"
fi


# Test #2
# Test that mail user@bad-domain will result in a warning from the mailer
# daemon that the domain does not exist. 

export TCID=mail02
export TST_COUNT=2
RC=0
RC1=0
RC2=0

$LTPBIN/tst_resm TINFO \
    "Test #2: mail user@bad-domain will result in failure"
$LTPBIN/tst_resm TINFO "Test #2: to deliver the mail. Mailer daemon should"
$LTPBIN/tst_resm TINFO "Test #2: report this failure."

if [ -z $MAIL_NOT_INSTALLED ]; then
cat > $LTPTMP/tst_mail.in <<EOF
This is a test email.
EOF

# Don't use underscores in domain names (they're illegal)...
mail -s "Test" root@thisdomaindoesnotexist < $LTPTMP/tst_mail.in \
     > $LTPTMP/tst_mail.out 2>&1
if [ $? -ne 0 ]
then
    $LTPBIN/tst_res TFAIL $LTPTMP/tst_mail.out \
        "Test #2: mail command failed. Reason: "
    TFAILCNT=$(( $TFAILCNT+1 ))
else
    # check if Mailer-Deamon reported any delivery failure.    
    # but wait for the mail to arrive first, sleep 5s.
    sleep 5s
    echo "d" | mail -u root > $LTPTMP/tst_mail.res 2>&1
    RC1=$(awk '/^>N/ {IGNORECASE=1; print match($3, "Mailer-Daemon")}' \
	$LTPTMP/tst_mail.res)
##################################################################
# In this testcase, mail will get "Returnedmail:", while mailx will
# get "UndeliveredMailReturned:".
# Either of mail and mailx may be linked to another.
# For example,
# /bin/mail -> /bin/mailx
# or
# /bin/mailx -> /bin/mail
##################################################################
     RC2=$(awk '/^>N/ {print match($9 $10, "Returnedmail:")}' \
             $LTPTMP/tst_mail.res)
     RC3=$(awk '/^>N/ {print match($9 $10, "UndeliveredMail")}' \
             $LTPTMP/tst_mail.res)
fi 
    if [ -z "$RC1" -a -z "$RC2" -a -z "$RC3" ]
    then
        $LTPBIN/tst_res TFAIL $LTPTMP/tst_mail.res \
        "Test #2: No new mail for root. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
    else
        if [ $RC1 -ne 0 -a $RC2 -ne 0 ] || [ $RC1 -ne 0 -a $RC3 -ne 0 ]
        then
            $LTPBIN/tst_resm TPASS \
                "Test #2: Mailer-Deamon reported delivery failure"
        else
            $LTPBIN/tst_res TFAIL $LTPTMP/tst_mail.res \
            "Test #2: Mailer-Deamon failed to report delivery failure. Reason:"
            TFAILCNT=$(( $TFAILCNT+1 ))
        fi
    fi
else
    $LTPBIN/tst_resm TCONF "mail command not installed"
fi    

# Test #3
# Test that mail non_existant_user@localhost will result in delivery failure.
# Mailer-Deamon will report this failure.

export TCID=mail03
export TST_COUNT=3
RC=0
RC1=0
RC2=0

$LTPBIN/tst_resm TINFO \
    "Test #3: mail non_existant_user@localhost will fail"
$LTPBIN/tst_resm TINFO "Test #3: to deliver the mail. Mailer daemon should"
$LTPBIN/tst_resm TINFO "Test #3: report this failure."

if [ -z $MAIL_NOT_INSTALLED ]; then
cat > $LTPTMP/tst_mail.in <<EOF
This is a test email.
EOF

mail -s "Test" non_existant_userr@localhost < $LTPTMP/tst_mail.in \
     > $LTPTMP/tst_mail.out 2>&1
if [ $? -ne 0 ]
then
    $LTPBIN/tst_res TFAIL $LTPTMP/tst_mail.out \
        "Test #3: mail command failed. Reason: "
    TFAILCNT=$(( $TFAILCNT+1 ))
else
    # check if Mailer-Deamon reported any delivery failure.    
    # but wait for the mail to arrive first, sleep 5s.
    sleep 5s
    echo "d" | mail -u root > $LTPTMP/tst_mail.res 2>&1
    RC1=$(awk '/^>N/ {IGNORECASE=1; print match($3, "Mailer-Daemon")}' \
	$LTPTMP/tst_mail.res)
##################################################################
# In this testcase, mail will get "Returnedmail:", while mailx will
# get "UndeliveredMailReturned:".
# Either of mail and mailx may be linked to another.
# For example,
# /bin/mail -> /bin/mailx
# or
# /bin/mailx -> /bin/mail
##################################################################
     RC2=$(awk '/^>N/ {print match($9 $10, "Returnedmail:")}' \
             $LTPTMP/tst_mail.res)
     RC3=$(awk '/^>N/ {print match($9 $10, "UndeliveredMail")}' \
             $LTPTMP/tst_mail.res)
fi
    if [ -z "$RC1" -a -z "$RC2" -a -z "$RC3" ]
    then
        $LTPBIN/tst_res TFAIL $LTPTMP/tst_mail.res \
        "Test #2: No new mail for root. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
    else
        if [ $RC1 -ne 0 -a $RC2 -ne 0 ] || [ $RC1 -ne 0 -a $RC3 -ne 0 ]
	then
            $LTPBIN/tst_resm TPASS \
                "Test #3: Mailer-Deamon reported delivery failure"
        else
            $LTPBIN/tst_res TFAIL $LTPTMP/tst_mail.res \
            "Test #3: Mailer-Deamon failed to report delivery failure. Reason:"
            TFAILCNT=$(( $TFAILCNT+1 ))
        fi
    fi

else
    $LTPBIN/tst_resm TCONF "mail command not installed"
fi

# Test #4 
# Test that mail -c user@domain option will carbon copy that user.

export TCID=mail04
export TST_COUNT=4
RC=0

$LTPBIN/tst_resm TINFO "Test #4: Test that mail -c user@domain will"
$LTPBIN/tst_resm TINFO "Test #4: carbon copy user@domain"
if [ -z $MAIL_NOT_INSTALLED ]; then

# send mail to root and carbon copy mail_test 
mail -s "Test" root@localhost -c mail_test@localhost < \
    $LTPTMP/tst_mail.in > $LTPTMP/tst_mail.out 2>&1 || RC=$?
if [ $RC -ne 0 ]
then
     $LTPBIN/tst_res TFAIL $LTPTMP/tst_mail.out \
        "Test #4: mail command failed. Reason:"
    TFAILCNT=$(( $TFAILCNT+1 ))
else
    # Check if mail_test received the mail and 
    # also if root received the main copy of the email.
    sleep 5s
    echo "d" | mail -u root > $LTPTMP/tst_mail.res 2>&1
    RC1=$(awk '/^>N/ {print match($9, "Test")}' $LTPTMP/tst_mail.res)
    echo "d" | mail -u mail_test > $LTPTMP/tst_mail.res 2>&1
    RC2=$(awk '/^>N/ {print match($9, "Test")}' $LTPTMP/tst_mail.res)
    if [ $RC1 -ne 0 -a $RC2 -ne 0 ]
    then
        $LTPBIN/tst_resm TPASS \
            "Test #4: Mail was carbon copied to user mail_test"
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/tst_mail.res \
            "Test #4: mail failed to carbon copy user mail_test. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
    fi
fi
else
    $LTPBIN/tst_resm TCONF "mail command not installed"
fi

# Test #5 
# Test that mail -b user@domain option will blind carbon copy that user.

export TCID=mail05
export TST_COUNT=5
RC=0

$LTPBIN/tst_resm TINFO "Test #5: Test that mail -b user@domain will"
$LTPBIN/tst_resm TINFO "Test #5: blind carbon copy user@domain"
if [ -z $MAIL_NOT_INSTALLED ]; then

# send mail to root and carbon copy mail_test 
mail -s "Test" root@localhost -c mail_test@localhost < \
    $LTPTMP/tst_mail.in > $LTPTMP/tst_mail.out 2>&1 || RC=$?
if [ $RC -ne 0 ]
then
     $LTPBIN/tst_res TFAIL $LTPTMP/tst_mail.out \
        "Test #5: mail command failed. Reason:"
    TFAILCNT=$(( $TFAILCNT+1 ))
else
    # Check if mail_test received the mail and 
    # also if root received the main copy of the email.
    sleep 5s
    echo "d" | mail -u root > $LTPTMP/tst_mail.res 2>&1
    RC1=$(awk '/^>N/ {print match($9, "Test")}' $LTPTMP/tst_mail.res)
    echo "d" | mail -u mail_test > $LTPTMP/tst_mail.res 2>&1
    RC2=$(awk '/^>N/ {print match($9, "Test")}' $LTPTMP/tst_mail.res)
    if [ $RC1 -ne 0 -a $RC2 -ne 0 ]
    then
        $LTPBIN/tst_resm TPASS \
            "Test #5: Mail was carbon copied to user mail_test"
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/tst_mail.res \
            "Test #5: mail failed to carbon copy user mail_testi. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
    fi
fi
else
    $LTPBIN/tst_resm TCONF "mail command not installed"
fi

#CLEANUP & EXIT
# remove all the temporary files created by this test.
export TCID=CLEANUP
export TST_COUNT=1

$LTPBIN/tst_resm TINFO "Test CLEAN: Removing temporary files from $LTPTMP"
rm -fr $LTPTMP/tst_mail* 

$LTPBIN/tst_resm TINFO "Test CLEAN: Removing temporary user mail_test"
userdel -r mail_test > /dev/null 2>&1

exit $TFAILCNT
