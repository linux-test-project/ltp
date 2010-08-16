#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2007                 ##
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

pidns01
rc=$?
if [ $rc -ne 0 ]; then
	err_code=$rc
fi

pidns02
rc=$?
if [ $rc -ne 0 ] && [ -z $err_code ]; then
	err_code=$rc
fi

pidns04
rc=$?
if [ $rc -ne 0 ] && [ -z $err_code ]; then
	err_code=$rc
fi

pidns05
rc=$?
if [ $rc -ne 0 ] && [ -z $err_code ]; then
	err_code=$rc
fi

pidns06
rc=$?
if [ $rc -ne 0 ] && [ -z $err_code ]; then
	err_code=$rc
fi

pidns30
rc=$?
if [ $rc -ne 0 ] && [ -z $err_code ]; then
	err_code=$rc
fi

pidns31
rc=$?
if [ $rc -ne 0 ] && [ -z $err_code ]; then
	err_code=$rc
fi

pidns10
rc=$?
if [ $rc -ne 0 ] && [ -z $err_code ]; then
	err_code=$rc
fi

pidns12
rc=$?
if [ $rc -ne 0 ] && [ -z $err_code ]; then
	err_code=$rc
fi

pidns13
rc=$?
if [ $rc -ne 0 ] && [ -z $err_code ]; then
	err_code=$rc
fi

pidns16
rc=$?
if [ $rc -ne 0 ] && [ -z $err_code ]; then
	err_code=$rc
fi

pidns17
rc=$?
if [ $rc -ne 0 ] && [ -z $err_code ]; then
	err_code=$rc
fi

pidns20
rc=$?
if [ $rc -ne 0 ] && [ -z $err_code ]; then
	err_code=$rc
fi

# If any test failed then exit with the value error-code.
if ! [ -z $err_code ]; then
	exit $err_code
fi

