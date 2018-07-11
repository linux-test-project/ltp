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
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
################################################################################

echo "Running in:"
#rm -f print_caps
#cp $LTPROOT/testcases/bin/print_caps .
FIFOFILE="${TMPDIR:=/tmp}/caps_fifo"
export FIFOFILE
rm -f $FIFOFILE
mkfifo $FIFOFILE
chmod 777 $FIFOFILE
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

unlink $FIFOFILE
exit $exit_code
