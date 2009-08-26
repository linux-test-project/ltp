#!/bin/sh

################################################################################ 
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2008                 ##
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
## Author:      Veerendra <veeren@linux.vnet.ibm.com>                         ##
################################################################################ 


rc=0
exit_code=0

# Check the iproute2 version (aka "SnapShot")
IPROUTEV=`ip -V | cut -d ',' -f 2 | cut -d '-' -f 2 | sed -e 's/^ss//'`

# We need to strip leading 0s else bash thinks we're giving it octal numbers.
IPROUTEY=$(echo ${IPROUTEV:0:2} | sed -e 's/^0\+//') # Year
IPROUTEM=$(echo ${IPROUTEV:2:2} | sed -e 's/^0\+//') # Month
IPROUTED=$(echo ${IPROUTEV:4:2} | sed -e 's/^0\+//') # Day

V=$((${IPROUTEY}*12*32 + ${IPROUTEM}*32 + ${IPROUTED}))

#
# iproute-ss080725 and later support setting the network namespace of an
# interface.
#
NETNSV=$((8*12*32 + 7*32 + 25))
if [ ${V} -lt ${NETNSV} ]; then
	echo "INFO: iproute tools do not support setting network namespaces. Skipping network namespace tests."
	exit $exit_code
fi

crtchild
rc=$?
if [ $rc -ne 0 ]; then
    exit_code=$rc
    errmesg="crtchild: return code is $exit_code ; "
    echo $errmesg
else
   echo "crtchild: PASS"
fi
echo

two_children_ns
rc=$?
if [ $rc -ne 0 ]; then
    exit_code=$rc
    errmesg="$errmesg two_children_ns: return code is $exit_code ; "
    echo $errmesg
else
   echo "two_children_ns: PASS"
fi
echo

crtchild_delchild
rc=$?
if [ $rc -ne 0 ]; then
    exit_code=$rc
    errmesg="$errmesg crtchild_delchild: return code is $exit_code ; "
    echo $errmesg
else
   echo "crtchild_delchild: PASS"
fi
echo


par_chld_ipv6
rc=$?
if [ $rc -ne 0 ]; then
    exit_code=$rc
    errmesg="$errmesg par_chld_ipv6: return code is $exit_code ; "
    echo $errmesg
else
   echo "par_chld_ipv6: PASS"
fi
echo

# sysfs tagging does not exist, so this test can't pass.  In
# fact at the moment it fails when mount -t sysfs none /sys is
# refused, fails in a bad state, leaving the system hard to
# reboot.  Revisit enabling this test when per-container sysfs
# views are supported.
#sysfsview
#rc=$?
#if [ $rc -ne 0 ]; then
#    exit_code=$rc
#    errmesg="$errmesg sysfsview: return code is $exit_code ; "
#    echo $errmesg
#else
#   echo "sysfsview: PASS"
#fi
#echo

par_chld_ftp
rc=$?
if [ $rc -ne 0 ]; then
    exit_code=$rc
    errmesg="$errmesg par_chld_ftp: FAIL $exit_code ; "
    echo $errmesg
else
   echo "par_chld_ftp: PASS"
fi
echo
exit $exit_code
