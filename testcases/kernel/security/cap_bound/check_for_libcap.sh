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

if [ "x$CC" = "x" ]; then
	export CC=gcc
fi
yesno=0
if [ "$1" = "yesno" ]; then
	yesno=1
fi

$CC -o dummy dummy.c -lcap 2>/dev/null

ret=$?

if [ $ret -ne 0 ]; then
	if [ $yesno -eq 1 ]; then
		echo no
	else
		exit 1
	fi
else
	if [ $yesno -eq 1 ]; then
		echo yes
	else
		exit 0
	fi
fi
