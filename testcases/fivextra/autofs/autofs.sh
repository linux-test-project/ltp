#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		      ##
##									      ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MEECHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	autofs.sh
#
# Description:	Test autofs package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
# History:	July 23 2003 - Created - Andrew Pham
#		Oct 10  2003 - Fixed up a bit to work better on ppc32.
#
#		06 Jan 2004 - (apham) updated to tc_utils.source
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source
tc_setup

TST_TOTAL=1
REQUIRED="automount ls grep ps sleep"
MFS="$TCTMP/mountpoint"
AUTOFILE="/etc/auto.mfile$$"
MOVED=0

function tc_local_cleanup()
{
	[ $MOVED -eq 1 ] && mv /etc/auto.master.save /etc/auto.master
	rm /etc/auto.mfile* >& /dev/null
}

################################################################################
# testcase functions
################################################################################
function TC_autofs()
{	
	tc_register "automount"

	/etc/init.d/autofs stop

	# setup
	if [ -e /etc/auto.master ]; then
		mv /etc/auto.master /etc/auto.master.save
		MOVED=1
	fi	

	echo "test   -fstype=iso9660,ro,loop :$LTPBIN/image.img" > $AUTOFILE

#	/etc/init.d/autofs reload

	# begin testing
	automount $MFS file $AUTOFILE > $stdout 2>$stderr
	tc_fail_if_bad $? "rcautofs start is not working." || return

	# to get the fs to mount
	ls $MFS/test >& /dev/null


	sleep 3

	[ -e $MFS/test/file1.txt -a -e $MFS/test/file2.txt ] &&
        [ -e $MFS/test/file3.txt -a -e $MFS/test/d2/klmnopqr.sys ] &&
        [ -e $MFS/test/d2/12345678  -a -e $MFS/test/d2/abcdefgh ]
	tc_pass_or_fail $? "$MFS/test is not mounted correctly."
	return
}
################################################################################
# main
################################################################################
tc_root_or_break || exit 

# Check if supporting utilities are available
tc_exec_or_break  $REQUIRED || exit

TC_autofs

#Stop automount
killall automount >& /dev/null 
if [ $? -ne 0 ]; then
	ps -ef | grep automount | grep -v grep
	echo
	echo "*********************************"
	echo "   The above messages displayed because "
	echo "      automount is still running."
	echo "*********************************"
fi	
