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
#                           - Added - Test #2.
#               Jan 03 2003 - Added - Test #3.
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

# Test #1
# Test that eject -d lists the default device. 

export TCID=eject01
export TST_COUNT=1

$LTPBIN/tst_resm TINFO "Test #1: eject -d will list the default device."

eject -d &>$LTPTMP/tst_eject.res || RC=$?
if [ $RC -eq 0 ]
then
    grep "eject: default device:" $LTPTMP/tst_eject.res \
        &>$LTPTMP/tst_eject.out || RC1=$?
    grep "cdrom" $LTPTMP/tst_eject.res \
        2>&1 1>>$LTPTMP/tst_eject.out  || RC2=$?
    if [[ $RC1 -eq 0 && $RC2 -eq 0 ]]
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


# Test #2
# Test that eject -d lists the default device. 

export TCID=eject02
export TST_COUNT=2
RC=0

$LTPBIN/tst_resm TINFO "Test #2: eject commad with no options"
$LTPBIN/tst_resm TINFO "Test #2: will eject the default cdrom device."

eject &>$LTPTMP/tst_eject.res || RC=$?
if [ $RC -eq 0 ]
then
    $LTPBIN/tst_resm TPASS  "Test #2: eject succeded"
else
    echo "Error code returned by eject: $RC" >> $LTPTMP/tst_eject.res \
        2&/dev/null
    $LTPBIN/tst_res TFAIL $LTPTMP/tst_eject.res \
        "Test #2: eject failed. Reason:"
    TFAILCNT=$((TFAILCNT+1))
fi

# Test #3
# Test the eject command will eject the default cdrom device and also unmount
# device if it is currently mounted.

export TCID=eject03
export TST_COUNT=3

$LTPBIN/tst_resm TINFO "Test #3: eject command will eject the default cdrom"
$LTPBIN/tst_resm TINFO "Test #3: device and also unmount the device if it"
$LTPBIN/tst_resm TINFO "Test #3: is currently mounted."

cp /etc/fstab $LTPTMP/fstab.bak &>/dev/null
mkdir $LTPTMP/cdrom &>/dev/null

echo "/dev/cdrom $LTPTMP/cdrom iso9660 defaults,ro,user,noauto 0 0" \
    >> /etc/fstab 2>/dev/null

mount $LTPTMP/cdrom &>$LTPTMP/tst_eject.out || RC=$?
if [ $RC -eq 0 ]
then
    echo ".Failed to mount $LTPTMP/cdrom." >> $LTPTMP/tst_eject.out 2>/dev/null
    $LTPBIN/tst_brk TBROK tst_eject.out "Test #3: mount failed. Reason:"
    TFAILCNT=$((TFAILCNT+1))
else
    eject &>$LTPTMP/tst_eject.out || RC=$?
    if [ $RC -eq 0 ]
    then 
        mount &>$LTPTMP/tst_eject.res
        grep "$LTPTMP/cdrom" $LTPTMP/tst_eject.res &>$LTPTMP/tst_eject.out \
            || RC=$?
        if [ $RC -ne 0 ]
        then
            $LTPBIN/tst_resm TPASS  "Test #3: eject unmounted device"
        else
            $LTPBIN/tst_resm TFAIL \
                "Test #3: eject failed to unmount /dev/cdrom."
            TFAILCNT=$((TFAILCNT+1))
        fi
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/tst_eject.out \
           "Test #3: eject failed. Reason:"
        TFAILCNT=$((TFAILCNT+1))
    fi
fi
 
if [ -f $LTPTMP/fstab.bak ]
then
    cp $LTPTMP/fstab.bak /etc/fstab &>/dev/null
else
    $LTPBIN/tst_res TINFO "Test #3: Could not restore /etc/fstab coz"
    $LTPBIN/tst_res TINFO "Test #3: backup file $LTPTMP/fstab.bak was lost!"
fi

#CLEANUP & EXIT
# remove all the temporary files created by this test.
rm -f $LTPTMP/tst_eject*
exit $TFAILCNT
