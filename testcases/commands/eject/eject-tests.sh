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
# File :        eject_tests.sh
#
# Description:  Tests basic functionality of eject command. 
#
# Author:       Manoj Iyer, manjo@mail.utexas.edu
#
# History:      Jan 01 2003 - Created - Manoj Iyer.
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

# Test #1
# Test that eject -d lists the default device. 

export TCID=logrotate01
export TST_COUNT=1

$LTPBIN/tst_resm TINFO "Test #1: eject -d will list the default device."

eject -d &>$LTPTMP/tst_eject.res || RC=$?
if [ $RC -eq 0 ]
then
    grep "eject: default device:" $LTPTMP/tst_eject.res \
        &>$LTPTMP/tst_eject.out || RC=$?
    if [ $RC -eq 0 ]
    then 
        $LTPBIN/tst_resm TPASS  "Test #1: eject -d lists the default device"
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/tst_eject.out \
            "Test #1: eject -d failed to list. Reason:"
        TFAILCNT=$((TFAILCNT+1))
    fi
else
    echo "return code from eject = $RC" > $LTPTMP/tst_eject.out 2>/dev/null
    $LTPBIN/tst_res TFAIL $LTPTMP/tst_eject.out \
        "Test #1: eject failed. Reason: "
fi


#CLEANUP & EXIT
# remove all the temporary files created by this test.
rm -f $LTPTMP/tst_eject*

exit $TFAILCNT
