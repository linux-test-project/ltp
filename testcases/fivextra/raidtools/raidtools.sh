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
# File :	raidtools.sh
#
# Description:	Test raidtools package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
# History:	April 08 2003 - Created - Andrew Pham
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
################################################################################
commands="mkraid raid0run raidstop raidstart"

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TST_TOTAL=4
REQUIRED="mkraid raidstart raidstop raid0run chmod cat \
	touch mount umount mke2fs which echo rm"

part1=$1
part2=$2

# Initialize output messages
ErrMsg="Failed: Not available."
ErrMsg1="Failed: Unexpected output.  Expected:"
################################################################################
# testcase functions
################################################################################
function TC_mkraid()
{	
	local CONFF=/etc/raidtab

	# Creating a conf file
	echo "raiddev	/dev/md0" > $CONFF
	echo "raid-level linear" >> $CONFF
	echo "chunk-size 16" >> $CONFF
	echo "nr-raid-disks 2" >> $CONFF
	echo "nr-spare-disks 0" >> $CONFF
	echo "persistent-superblock 0" >> $CONFF
	echo "device $part1" >> $CONFF
	echo "raid-disk 0" >> $CONFF
	echo "device $part2" >> $CONFF
	echo "raid-disk 1" >> $CONFF

	# Make sure /dev/md0 is not active
	raidstop /dev/md0 >&/dev/null
	
	mkraid /dev/md0 >$stdout 2>$stderr
	tc_fail_if_bad $? "$ErrMsg" || exit
	
	# create a file system on our devices
	mke2fs $part1 >&/dev/null
	mke2fs $part2 >&/dev/null
	
	mkdir $TCTMP/temp >&/dev/null
	mount /dev/md0 $TCTMP/temp >&/dev/null
	touch $TCTMP/temp/f1 $TCTMP/temp/f2 $TCTMP/temp/f3 >&/dev/null

	local expect="$TCTMP/temp/f1 $TCTMP/temp/f2 $TCTMP/temp/f3"
	[ -e $TCTMP/temp/f1 -a -e $TCTMP/temp/f2 -a -e $TCTMP/temp/f3 ]
	tc_pass_or_fail $? "$ErrMsg1 $expect to exist." || exit

	rm -fr $TCTMP/temp >&/dev/null
	umount $TCTMP/temp >&/dev/null
	raidstop /dev/md0 >&/dev/null
}

function TC_raid0run()
{
	raid0run /dev/md0 >$stdout 2>$stderr
	tc_fail_if_bad $? "$ErrMsg"

	if [ -d /proc ]; then
		grep active /proc/mdstat >&/dev/null
		tc_pass_or_fail $? "$ErrMsg1 /dev/md0 to be active" 
		raidstop /dev/md0 >&/dev/null
		return $?
	fi

	raidstop /dev/md0 >&/dev/null
	tc_pass_or_fail 0 "passed"
}

function TC_raidstop()
{
	[ raid0run /dev/md0 >&/dev/null ] || {
		tc_break_if_bad $? "Unable to start /dev/md0 ";
		return; }

	raidstop /dev/md0  >$stdout 2>$stderr
	tc_fail_if_bad $? "$ErrMsg"  

	if [ -d /proc ]; then
                grep -v active /proc/mdstat >&/dev/null
		tc_pass_or_fail $? "$ErrMsg1 /dev/md0 NOT to be active."
		return $?
	fi

	tc_pass_or_fail 0 "passed"
}

function TC_raidstart()
{
	raidstart /dev/md0 >&/dev/null
	if [ $? -eq 0 ]; then
		tc_pass_or_fail 1 "raidstart operates incorrectly; see /etc/raidtab"
		return 1
	fi
	if [ -d /proc ]; then
		grep -v active /proc/mdstat >&/dev/null
		tc_pass_or_fail $? "$ErrMsg1 /dev/md0 NOT to be active."
		return $?
	fi

	tc_pass_or_fail 0 "passed"
}
################################################################################

# main
################################################################################
tc_setup

# Make two test partitions are given
RC=1
mount -l | grep $part1 >&/dev/null
RC=$?
mount -l | grep $part2 >&/dev/null
RC1=$?
if test $# -ne 2 -o $RC1 -eq 0 -o $RC -eq 0 ; then
	tc_info "Please provide 2 unmounted partitions in senario file."
	exit 0
fi


# Check if supporting utilities are available
tc_exec_or_break  $REQUIRED || exit

E_value=0
for cmd in $commands
do
	tc_register "$cmd"
	TC_$cmd || E_value=$?
done
exit $E_value
