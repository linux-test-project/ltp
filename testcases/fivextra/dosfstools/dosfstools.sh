#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2001                 ##
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
# File :       dosfstools.sh
#
# Description: This testcase tests the functionalitis of the dosfstools package 
#
# commands:    /sbin/mkdosfs /sbin/dosfsck
#
# Author:       Andrew Pham, apham@us.ibm.com
#
# History:      Mar 19 2003 - Created -Andrew Pham.
#		Apr 18 2003 - Edited description and combined 2 tests into one
#				 Andrew Pham
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TST_TOTAL=1
REQUIRED="mkdosfs dosfsck mount umount touch grep ls"

################################################################################
# the testcase functions
################################################################################
function TC_mkdosfs()
{
	
	mkdosfs -C -F 32 $TCTMP/mm_mount.img 1024 \
		>/dev/null 2>/$stderr
	tc_fail_if_bad $?  "$summary2" || return 1
	
	mkdir $TCTMP/mnt >&/dev/null
	if ls -l `which ls` | grep busybox >&/dev/null; then
		tc_exec_or_break modprobe || return 1
		modprobe vfat >&/dev/null	
		mount $TCTMP/mm_mount.img $TCTMP/mnt -t vfat -o loop >&/dev/null
	else		
		mount $TCTMP/mm_mount.img $TCTMP/mnt -o loop >&/dev/null
	fi
	
	touch $TCTMP/mnt/f1 $TCTMP/mnt/f2 $TCTMP/mnt/f3 >&/dev/null
	umount $TCTMP/mnt
	
	dosfsck $TCTMP/mm_mount.img | grep mm_mount.img | grep 3 >&/dev/null
	tc_pass_or_fail $?  "Unable to use the newly created FAT32 filesystem"

}
################################################################################
# main
################################################################################
tc_setup

[ "$TCTMP" ] && rm -rf $TCTMP/*

# Check if supporting utilities are available
tc_exec_or_break  $REQUIRED || exit 1

tc_register "mkdosfs and dosfsck" 
TC_mkdosfs
