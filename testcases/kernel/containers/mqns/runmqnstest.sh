#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2009                 ##
## Copyright (c) Nadia Derbey, 2009                                           ##
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

exit_code=0
tests_list='mqns_01 mqns_02 mqns_03 mqns_04'

for t in $tests_list
do
	$t
	if [ $? -ne 0 ]; then
		exit_code="$?"
		exit $exit_code
	fi
	$t -clone
	if [ $? -ne 0 ]; then
		exit_code="$?"
		exit $exit_code
	fi
done

exit $exit_code
