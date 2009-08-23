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
# File :        eject_tests.sh
#
# Description:  Tests basic functionality of eject command. 
#
# Author:       Manoj Iyer, manjo@mail.utexas.edu
#
# History:      Jan 01 2003 - Created - Manoj Iyer.
#                           - Added - Test #2.
#               Jan 03 2003 - Added - Test #3.
#               Jan 06 2003 - Modified - Test #3.
#                           - Changed tst_brk to use correct parameters.
#                           - Check if $LTPTMP/cdrom directory exists before 
#                             creating it.
#                           - Corrected code to check if return code is not 0
#                             which indicated failure.
#                           - fixed code to add $LTPTMP/cdrom to /etc/fstab
#               Jan 07 2003 - Call eject with -v for verbose information.
#                Jan 08 2003 - Added test #4.
#


export TST_TOTAL=4

if [ -z $LTPTMP && -z $TMPBASE ]
then
    LTPTMP=/tmp
else
    LTPTMP=$TMPBASE
fi

if [ -z $LTPBIN && -z $LTPROOT ]
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

eject -d > $LTPTMP/tst_eject.res 2>&1 || RC=$?
if [ $RC -eq 0 ]
then
    grep "eject: default device:" $LTPTMP/tst_eject.res \
        > $LTPTMP/tst_eject.out 2>&1 || RC1=$?
    grep "cdrom" $LTPTMP/tst_eject.res \
        2>&1 1>>$LTPTMP/tst_eject.out  || RC2=$?
    if [ $RC1 -eq 0 ] && [ $RC2 -eq 0 ]
    then 
        $LTPBIN/tst_resm TPASS  "Test #1: eject -d lists the default device"
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/tst_eject.out \
            "Test #1: eject -d failed to list. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
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

$LTPBIN/tst_resm TINFO "Test #2: eject command with no options"
$LTPBIN/tst_resm TINFO "Test #2: will eject the default cdrom device."

eject -v > $LTPTMP/tst_eject.res 2>&1 || RC=$?
if [ $RC -eq 0 ]
then
    grep "CD-ROM eject command succeeded" $LTPTMP/tst_eject.res \
        > $LTPTMP/tst_eject.out 2>&1 || RC=$?
    if [ $RC -eq 0 ]
    then
        # Close the tray if it is supported.
        eject -t > /dev/null 2>&1
        $LTPBIN/tst_resm TPASS  "Test #2: eject succeded"
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/tst_eject.out  \
            "Test #2: eject fail.  Reason"
    fi
else
    echo "Error code returned by eject: $RC" >>$LTPTMP/tst_eject.res \
        2&/dev/null
    $LTPBIN/tst_res TFAIL $LTPTMP/tst_eject.res \
        "Test #2: eject failed. Reason:"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi


# Test #3
# Test the eject command will eject the default cdrom device and also unmount
# device if it is currently mounted.

export TCID=eject03
export TST_COUNT=3
RC=0

$LTPBIN/tst_resm TINFO "Test #3: eject command will eject the default cdrom"
$LTPBIN/tst_resm TINFO "Test #3: device and also unmount the device if it"
$LTPBIN/tst_resm TINFO "Test #3: is currently mounted."

cp /etc/fstab $LTPTMP/fstab.bak > /dev/null 2>&1

if [ -d $LTPTMP/cdrom ]
then
    $LTPBIN/tst_resm TINFO \
        "Test #3: test cdrom mount point $LTPTMP/cdrom exists. Skip creation" 
else
    mkdir -p $LTPTMP/cdrom > $LTPTMP/tst_eject.out 2>&1 || RC=$?
    if [ $RC -ne 0 ]
    then
        $LTPBIN/tst_brk TBROK $LTPTMP/tst_eject.out NULL \
            "Test #3: failed to make directory $LTPTMP/cdrom. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
    fi
fi

echo "/dev/cdrom $LTPTMP/cdrom iso9660 defaults,ro,user,noauto 0 0" >>/etc/fstab 2>$LTPTMP/tst_eject.out || RC=$?
if [ $RC -ne 0 ]
then
    $LTPBIN/tst_brk TBROK $LTPTMP/tst_eject.out NULL \
        "Test #3: failed adding $LTPTMP/cdrom to /etc/fstab. Reason:"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi

mount $LTPTMP/cdrom > $LTPTMP/tst_eject.out 2>&1 || RC=$?
if [ $RC -ne 0 ]
then
    echo ".Failed to mount $LTPTMP/cdrom." >> $LTPTMP/tst_eject.out 2>/dev/null
    $LTPBIN/tst_brk TBROK $LTPTMP/tst_eject.out NULL \
             "Test #3: mount failed. Reason:"
    TFAILCNT=$(( $TFAILCNT+1 ))
else
    eject > $LTPTMP/tst_eject.out 2>&1 || RC=$?
    if [ $RC -eq 0 ]
    then 
        mount > $LTPTMP/tst_eject.res 2>&1
        grep "$LTPTMP/cdrom" $LTPTMP/tst_eject.res > $LTPTMP/tst_eject.out 2>&1 \
            || RC=$?
        if [ $RC -ne 0 ]
        then
            $LTPBIN/tst_resm TPASS  "Test #3: eject unmounted device"
        else
            $LTPBIN/tst_resm TFAIL \
                "Test #3: eject failed to unmount /dev/cdrom."
            TFAILCNT=$(( $TFAILCNT+1 ))
        fi
    else
        $LTPBIN/tst_res TFAIL $LTPTMP/tst_eject.out \
           "Test #3: eject failed. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
    fi
fi

if [ -f $LTPTMP/fstab.bak ]
then
    mv $LTPTMP/fstab.bak /etc/fstab > /dev/null 2>&1
else
    $LTPBIN/tst_resm TINFO "Test #3: Could not restore /etc/fstab coz"
    $LTPBIN/tst_resm TINFO "Test #3: backup file $LTPTMP/fstab.bak was lost!"
fi


# Test #4
# Test if eject -a on|1|off|0 will enable/disable auto-eject mode 
# the drive automatically ejects when the device is closed.

export TCID=eject04
export TST_COUNT=4
RC=0

$LTPBIN/tst_resm TINFO "Test #4: eject -a on|1|off|0 will "
$LTPBIN/tst_resm TINFO "Test #4: enable/disable auto-eject mode"
$LTPBIN/tst_resm TINFO "Test #4: NOTE!!! Some devices do not support this mode"
$LTPBIN/tst_resm TINFO "Test #4: so test may fail."

# Check is temporary mount point for /dev/cdrom exists
# if not create one.
if [ -d $LTPTMP/cdrom ]
then
    $LTPBIN/tst_resm TINFO "$LTPTMP/cdrom exists, skip creating the directory" 
else
    mkdir -p $LTPTMP/cdrom > $LTPTMP/tst_eject.out 2>&1 || RC=$?
    if [ $RC -ne 0 ]
    then
        $LTPBIN/tst_brk TBROK $LTPTMP/tst_eject.out NULL \
            "Test #3: failed to make directory $LTPTMP/cdrom. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
    fi
fi

# Check if /etc/fstab has this temporary mount point for /dev/cdrom listed 
# as one of the entries. If not create and entry and make a back up of the 
# origianl /etc/fstab
grep "$LTPTMP/cdrom" /etc/fstab > $LTPTMP/tst_eject.out 2>&1 || RC=$?
if [ -f $LTPTMP/fstab.bak && $RC -eq 0 ]
then
     $LTPBIN/tst_resm TINFO "$LTPTMP/cdrom entry exists in /etc/fstab" 
else
    cp /etc/fstab $LTPTMP/fstab.bak > $LTPTMP/tst_eject.out 2>&1
    echo "/dev/cdrom $LTPTMP/cdrom iso9660 defaults,ro,user,noauto 0 0" >>/etc/fstab 2>$LTPTMP/tst_eject.out || RC=$?
    if [ $RC -ne 0 ]
    then
        $LTPBIN/tst_brk TBROK $LTPTMP/tst_eject.out NULL \
        "Test #4: failed adding $LTPTMP/cdrom to /etc/fstab. Reason:"
        TFAILCNT=$(( $TFAILCNT+1 ))
    fi
fi

# mount the cdrom device /dev/cdrom on to $LTPTMP/cdrom 
# and enable auto-eject. unmounting $LTPTMP/cdrom should open the tray and
# eject the cdrom.

mount $LTPTMP/cdrom > $LTPTMP/tst_eject.out 2>&1 || RC=$?
if [ $RC -ne 0 ]
then
    $LTPBIN/tst_brk TBROK $LTPTMP/tst_eject.out NULL \
        "Test #4: failed mounting $LTPTMP/cdrom. Reason: "
    TFAILCNT=$(( $TFAILCNT+1 ))
fi

eject -a 1 > $LTPTMP/tst_eject.out 2>&1 || RC=$?
if [ $RC -ne 0 ]
then
    $LTPBIN/tst_res TFAIL $LTPTMP/tst_eject.out NULL \
        "Test #4: eject command failed setting auto-eject mode on. Reason:"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi

# check if the tray is still closed and not open.
# check_tray will return 2 if the tray is open.

$LTPBIN/check_tray || RC=$?
if [ $RC -eq 2 ]
then
    $LTPBIN/tst_brkm TBROK NULL \
        "Test #4: /dev/cdrom is mounted but the tray is open!"
    TFAILCNT=$(( $TFAILCNT+1 ))
fi

# closing the device i.e unmounting $LTPTMP/cdrom should now open the tray
# i.e auto-eject the cdrom.

umount $LTPTMP/cdrom > $LTPTMP/tst_eject.out 2>&1 || RC=$?
if [ $RC -ne 0 ]
then
    $LTPBIN/tst_brk TBROK $LTPTMP/tst_eject.out NULL \
        "Test #4: unmounting the cdrom failed. Reason: "
    TFAILCNT=$(( $TFAILCNT+1 ))
else
    $LTPBIN/check_tray || RC=$?
    if [ $RC -eq 2 ]
    then
        $LTPBIN/tst_resm TPASS  "Test #4: /dev/cdrom is tray is open"
    else
        $LTPBIN/tst_resm TFAIL "Test #4: /dev/cdrom is tray is still closed" 
    fi
fi

# disable auto-eject, closing the device should not open the tray.

eject -a 0 > $LTPTMP/tst_eject.out 2>&1 || RC=$?
if [ $RC -ne 0 ]
then
    $LTPBIN/tst_res TFAIL $LTPTMP/tst_eject.out NULL \
        "Test #4: eject command failed setting auto-eject mode on. Reason:"
    TFAILCNT=$(( $TFAILCNT+1 ))
else
    $LTPBIN/tst_resm TINFO "Test #4: auto-eject feature disabled"
fi

# close the tray

eject -tv > $LTPTMP/tst_eject.res 2>&1 || RC=$?
if [ $RC -ne 0 ]
then
    $LTPBIN/tst_res TFAIL $LTPTMP/tst_eject.res NULL \
        "Test #4: eject command to close the tray. Reason:"
    TFAILCNT=$(( $TFAILCNT+1 ))
else
    grep "closing tray" $LTPTMP/tst_eject.res > $LTPTMP/tst_eject.out 2>&1 || RC=$?    
    if [ $RC -eq 0 ]
    then
        $LTPBIN/check_tray || RC=$?
        if [ $RC -eq 2 ]
        then 
            $LTPBIN/tst_brkm TBROK NULL \
                "Test #4: eject -t reported tray closed, but tray is open"
            TFAILCNT=$(( $TFAILCNT+1 )) 
        fi
    fi
fi

mount $LTPTMP/cdrom > $LTPTMP/tst_eject.out 2>&1 || RC=$?
if [ $RC -ne 0 ]
then
    $LTPBIN/tst_brk TBROK $LTPTMP/tst_eject.out NULL \
        "Test #4: failed mounting $LTPTMP/cdrom. Reason: "
    TFAILCNT=$(( $TFAILCNT+1 ))
fi

umount $LTPTMP/cdrom > $LTPTMP/tst_eject.out 2>&1 || RC=$?
if [ $RC -ne 0 ]
then
    $LTPBIN/tst_brk TBROK $LTPTMP/tst_eject.out NULL \
        "Test #4: failed mounting $LTPTMP/cdrom. Reason: "
    TFAILCNT=$(( $TFAILCNT+1 ))
fi

$LTPBIN/check_tray || RC=$?
if [ $RC -eq 2 ]
then
    $LTPBIN/tst_resm TFAIL \
        "Test #4: closing the device opened the tray, but, auto-eject = off"
    TFAILCNT=$(( $TFAILCNT+1 ))
else
    $LTPBIN/tst_resm TPASS "Test #4: eject can enable and disable auto-eject"
fi


if [ -f $LTPTMP/fstab.bak ]
then
    mv $LTPTMP/fstab.bak /etc/fstab > /dev/null 2>&1
else
    $LTPBIN/tst_resm TINFO "Test #4: Could not restore /etc/fstab coz"
    $LTPBIN/tst_resm TINFO "Test #4: backup file $LTPTMP/fstab.bak was lost!"
fi

#CLEANUP & EXIT
# remove all the temporary files created by this test.
rm -fr $LTPTMP/tst_eject* $LTPTMP/cdrom
eject -t > /dev/null 2>&1

exit $TFAILCNT
