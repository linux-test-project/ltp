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
# Description:  Tests basic mail command.
#
# Author:       Manoj Iyer, manjo@mail.utexas.edu
#
# History:      Jan 07 2003 - Created - Manoj Iyer.
#
#! /bin/sh


export TST_TOTAL=1

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

# Test #1
# Test that mail user@domain will send a mail to that user at that domain.

export TCID=mail01
export TST_COUNT=1

$LTPBIN/tst_resm TINFO "Test #1: mail root@localhost will send mail to root"
$LTPBIN/tst_resm TINFO "Test #1: user on local machine."


cat > $LTPTMP/tst_mail.in <<EOF
This is a test email.
EOF

mail -s "Test" root@localhost < $LTPTMP/tst_mail.in \
	&>$LTPTMP/tst_mail.out || RC=$?
if [ $RC -ne 0 ]
then
	$LTPBIN/tst_res TFAIL $LTPTMP/tst_mail.out \
		"Test #1: mail command failed. Reason: "
    TFAILCNT=$((TFAILCNT+1))
else
	# check if root received a new email with Test as subject
	# but wait for the mail to arrive.
	
	sleep 10s
	echo "d" | mail -u root &>$LTPTMP/tst_mail.res
	mailsub=$(awk '/^>N/ {print $9}' $LTPTMP/tst_mail.res)
	if [ $mailsub == "\"Test\"" ]
	then
		$LTPBIN/tst_resm TPASS "Test #1: Mail was send to root & was received"
	else
		$LTPBIN/tst_res TFAIL $LTPTMP/tst_mail.res \
			"Test #1: Mail send to root, but was not received"
		TFAILCNT=$((TFAILCNT+1))
	fi
fi

#CLEANUP & EXIT
# remove all the temporary files created by this test.
#rm -fr $LTPTMP/tst_mail* 

exit $TFAILCNT
