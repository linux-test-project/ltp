#!/bin/bash

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
## Author:      Veerendra <veeren@linux.vnet.ibm.com>                         ##
################################################################################

export TCID=${TCID:-netns_par_chld_ftp.sh}
export TST_COUNT=1
export TST_TOTAL=1

. test.sh
. daemonlib.sh

flag=0

status_daemon vsftpd
if [ $? -ne 0 ]; then
	start_daemon vsftpd
	if [ $? -ne 0 ]; then
		TST_CLEANUP=""
		tst_brkm TCONF "Can't start vsftp"
	fi
	flag=1
fi

netns_par_chld_ftp &
tst_record_childstatus $!

if [ $flag -eq 1 ]; then
	stop_daemon vsftpd
fi

tst_exit
