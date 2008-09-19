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

sysfsview
rc=$?
if [ $rc -ne 0 ]; then
    exit_code=$rc
    errmesg="$errmesg sysfsview: return code is $exit_code ; "
    echo $errmesg
else
   echo "sysfsview: PASS"
fi
echo

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
