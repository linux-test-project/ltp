#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003						      ##
##									      ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	eject_tests.sh
#
# Description:	Tests basic functionality of eject command. 
#
# Author:	Manoj Iyer, manjo@mail.utexas.edu
#
# History:	Jan 01 2003 - Created - Manoj Iyer.
#			- Added - Test #2.
#		Jan 03 2003 - Added - Test #3.
#		Jan 06 2003 - Modified - Test #3.
#			- Changed tst_brk to use correct parameters.
#			- Check if $TCTMP/cdrom directory tc_exist_or_break before 
#			  creating it.
#			- Corrected code to check if return code is not 0
#			  which indicated failure.
#			- fixed code to add $TCTMP/cdrom to /etc/fstab
#		Jan 07 2003 - Call eject with -v for verbose information.
#		Jan 08 2003 - Added test #4.
#		Aug 15 2003 - Modified to use tc_utils.source.
#		09 Dec 2003 (rcp) updated to tc_utils.source
#		23 Jan 2004 (rcp) fixed bug in test03. final mount command
#				should NOT show cdrom.
#				TODO: test04 thinks it can't close drive
#				when actually it can.

################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source 

################################################################################
# utility functions specific to this script
################################################################################

#
# testcase local cleanup
#
function tc_local_cleanup()
{
	
	[ -e $TCTMP/fstab.bak ] && \
	{ mv $TCTMP/fstab.bak /etc/fstab &>/dev/null ; }
	eject -t &>/dev/null
}


#
# testcase local setup
#
function tc_local_setup()
{

	# check dependencies
	tc_root_or_break || exit
	tc_exec_or_break tray_check cp grep diff mount || exit

	# must have /dev/cdrom
	tc_exists /dev/cdrom || {
		tc_info "No /dev/cdrom found."
		tc_info "Perhaps you need something like \"ln -s /dev/hdc /dev/cdrom\""
		tc_exist_or_break /dev/cdrom
		return
	}

	# check if the cdrom media is present, if not present bail.
	tray_check &>/dev/null
	[ $? -ne 4 ] && \
	{ tc_break_if_bad $? "This test requires a cdrom media in the drive" || return ; }

	# mount point for cdrom
	mkdir -p $TCTMP/cdrom

	# save current fstab and add cdrom mount point
	cp /etc/fstab $TCTMP/fstab.bak
	echo "/dev/cdrom $TCTMP/cdrom iso9660 defaults,ro,user,noauto 0 0" >>/etc/fstab
}

################################################################################
# the test functions
################################################################################

# 
# test01	Test that eject -d lists the default device. 
# 
function test01()
{
	tc_register    "eject -d functionality"
	tc_info "eject -d will list the default device."
	
	eject -d &>$TCTMP/tst_eject.res
	tc_fail_if_bad $? "did not list default dev $(cat $TCTMP/tst_eject.res)" || \
	return

	for string in "eject" "default" "device" "cdrom"
	do
		grep $string $TCTMP/tst_eject.res 2>$stderr 1>$stdout
		tc_fail_if_bad $? "eject did not report default device."
	done
	tc_pass_or_fail 0	# always pass if we get this far
}

#
# test02	eject with no command options will eject the default device"
#
function test02()
{
	tc_register    "eject -v functionality"
	tc_info "eject commad with no options"
	tc_info "will eject the default cdrom device."
	
	eject -v &>$TCTMP/tst_eject.res 
	tc_fail_if_bad $? "failed to eject default device." || return
	
	# close the drive some drives dont support this so 
	# not checking for return code.
	eject -t &>/dev/null 

	grep "CD-ROM eject command succeeded" $TCTMP/tst_eject.res	\
	2>$stderr 1>$stdout 
	tc_pass_or_fail $? "did not eject default device"
}

#
# test03	Test the eject command will eject the default cdrom 
#				device and also unmount device if it is currently mounted.
function test03()
{
	tc_register    "eject  functionality"
	tc_info "eject command will eject the default cdrom"
	tc_info "device and also unmount the device if it"
	tc_info "is currently mounted."

	tray_check &>/dev/null
	[ $? -ne 4 ] && \
	{
		tc_warn "unable to close drive. Not supported. skip test"
		let TST_TOTAL--
		return 0;
	}

	mount $TCTMP/cdrom 2>$stderr 1>$stdout
	tc_fail_if_bad $? "failed mounting $TCTMP/cdrom" || return

	eject 2>$stderr 1>$stdout 
	tc_fail_if_bad $? "eject failed to eject default device" || return

	mount >$stdout 2>$stderr
	grep -qv "cdrom" $stdout
	tc_pass_or_fail $? "eject ejected the device but failed to unmount it"
}

#
# test04	Test if eject -a on|1|off|0 will enable/disable auto-eject mode 
#			the drive automatically ejects when the device is closed.
#
function test04()
{
	tc_register    "eject -a on|1|off|0 functionality"
	tc_info "eject -a on|1|off|0 will "
	tc_info "enable/disable auto-eject mode"
	tc_info "NOTE!!! Some devices do not support this mode"
	tc_info "so test may fail."

	tray_check &>/dev/null
	[ $? -ne 4 ] && \
	{
		tc_warn "unable to close drive. Not supported. skip test"
		return 0;
	}

	# mount the cdrom device /dev/cdrom on to $TCTMP/cdrom 
	mount $TCTMP/cdrom 2>$stderr 1>$stdout 
	tc_fail_if_bad $? "failed mounting cdrom device" || return
	
	# and enable auto-eject. 
	eject -a 1 2>$stderr 1>$stdout 
	tc_fail_if_bad $? "set auto-eject failed." || return
	
	# check if the tray is still closed and not open.
	# tray_check will return 2 if the tray is open.
	
	tray_check || RC=$?
	[ $RC -eq 2 ] && \
	tc_fail_if_bad $? "/dev/cdrom is mounted but the tray is open!" || return
	
	# closing the device i.e unmounting $TCTMP/cdrom should now open the tray
	# i.e auto-eject the cdrom.
	
	umount $TCTMP/cdrom 2>$stderr 1>$stdout
	tc_fail_if_bad $? "unmounting the cdrom failed." || return

	tray_check 
	[ $? -eq 2 ] && \
	tc_fail_if_bad $? "dev/cdrom is tray is still closed" || return
	
	# disable auto-eject, closing the device should not open the tray.
	
	eject -a 0 2>$stderr 1>$stdout 
	tc_fail_if_bad $? "failed setting auto-eject mode off" || return
	
	# close the tray
	eject -tv >$TCTMP/tst_eject.out 2>&1
	tc_fail_if_bad $? "failed to close the tray" || return

	# check if eject command reported the tray is closed
	grep "closing tray" $TCTMP/tst_eject.out 2>$stderr 1>$stdout
	tc_fail_if_bad $? "eject did not report that the tray is closed" || return

	# double check to see if the tray is really closed or open
	tray_check 
	[ $? -eq 2 ] && \
	tc_fail_if_bad $? "eject -t reported tray closed, but tray is open" || return
	
	mount $TCTMP/cdrom 2>$stderr 1>$stdout
	tc_fail_if_bad $? "failed mounting $TCTMP/cdrom" || return
	
	umount $TCTMP/cdrom 2>$stderr 1>$stdout
	tc_fail_if_bad $? "failed unmounting $TCTMP/cdrom" || return
	
	tray_check
	[ $? -eq 2 ] && \
	tc_fail_if_bad $? "closing the device opened the tray, auto-eject = off" ||    \
	return
	
	tc_pass_or_fail $? "auto-eject testcase failed"
	
}

#
# main
#

TST_TOTAL=4
tc_setup		# exits on failure

test01 &&
test02 &&
test03 &&
test04
