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
################################################################################

checkforlibcap
ret=$?
if [ $ret -ne 0 ]; then
	echo "Filecaps 0 CONF : System doesn't support execution of the test"
	echo setcap not installed.  Please install libcap-2.11 or newer from
	echo ftp://ftp.kernel.org/pub/linux/libs/security/linux-privs/libcap2
	exit 0
fi
checkforfilecaps.sh
ret=$?
if [ $ret -ne 0 ]; then
	exit 1
fi

echo "Running in:"
cp $LTPROOT/testcases/bin/print_caps .
mkfifo caps_fifo
chmod 777 caps_fifo
exit_code=0
echo "cap_sys_admin tests"
verify_caps_exec 0
tmp=$?
if [ $tmp -ne 0 ]; then
	exit_code=$tmp
fi

echo "testing for correct caps"
verify_caps_exec 1
tmp=$?
if [ $tmp -ne 0 ]; then
	exit_code=$tmp
fi

echo "testing for correct pI checks"
inh_capped
tmp=$?
if [ $tmp -ne 0 ]; then
	exit_code=$tmp
fi

exit $exit_code
