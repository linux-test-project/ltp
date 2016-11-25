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

if tst_kvcmp -lt "2.6.25"; then
	tst_resm TCONF "System kernel version is less than 2.6.25"
	tst_resm TCONF "Cannot execute test"
	exit 32
fi

echo "testing bounding set reading"
exit_code=0

cap_bounds_r
tmp=$?
if [ $tmp -ne 0 ]; then
	exit_code=$tmp
fi

echo "testing bounding set dropping"
cap_bounds_rw
tmp=$?
if [ $tmp -ne 0 ]; then
	exit_code=$tmp
fi

echo "checking bounding set constraint in pI"
cap_bset_inh_bounds
tmp=$?
if [ $tmp -ne 0 ]; then
	exit_code=$tmp
fi

exec_with_inh
tmp=$?
if [ $tmp -ne 0 ]; then
	exit_code=$tmp;
fi
exec_without_inh
tmp=$?
if [ $tmp -ne 0 ]; then
	exit_code=$tmp;
fi

exit $exit_code
