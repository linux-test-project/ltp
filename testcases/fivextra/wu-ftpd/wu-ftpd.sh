#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		                                      ##
##									      ##
## This program is free software;  you can redistribute it and/or modify      ##
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
# File :	wu-ftpd.sh
#
# Description:	Test the wu-ftpd package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Sep 16 2003 - created - RR
#		08 Jan 2004 - (RR) updated to tc_utils.source
#		21 Jan 2004 - (rcp) removed dependency on expect
#
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

#
# global variables
#
RQD_CMDS="cat grep cut ncftpget diff /etc/init.d/xinetd"
RQD_FILES="/etc/passwd"

################################################################################
# utility functions
################################################################################

#
# tc_local_setup
#
function tc_local_setup()
{
	tc_root_or_break || return
	tc_exec_or_break $RQD_CMDS || return
	tc_exist_or_break $RQD_FILES || return
	tc_add_user_or_break || return
}

################################################################################
# testcase functions
################################################################################

function test_01 {

	tc_register "Installation Check"

	tc_executes "ckconfig ftpcount ftpwho"
	tc_fail_if_bad "$?" "wu-ftpd executables not properly installed" || return
	tc_exists "/usr/sbin/in.ftpd"
	tc_pass_or_fail "$?" "wu-ftpd server is not properly installed"
}

function test_02 {

	tc_register "Test operation of wu-ftpd server"
	local SERVER="127.0.0.1"

	# array to parse ncftpget error codes into readable string
	errmsg[1]="ERROR: Could not connect to host."
	errmsg[2]="ERROR: Could not connect to host - timed out."
	errmsg[3]="ERROR: Transfer failed."
	errmsg[4]="ERROR: Transfer failed - timed out."
	errmsg[5]="ERROR: Directory change failed."
	errmsg[6]="ERROR: Directory change failed - timed out."
	errmsg[7]="ERROR: Malformed URL."
	errmsg[8]="ERROR: Usage error."
	errmsg[9]="ERROR: Error in login configuration file."
	errmsg[10]="ERROR: Library initialization failed."
	errmsg[11]="ERROR: Session initialization failed."

	# Create file to be downloaded in temp_user's home directory.
	local home_dir=$(cat /etc/passwd | grep "^$temp_user" | cut -f6 -d:)
	cat <<-EOF > $home_dir/test_file.txt
		this is a test file
	EOF

	# Be sure xinetd is running.
	/etc/init.d/xinetd restart 1>$stdout 2>$stderr
	tc_fail_if_bad "$?" "Unable to start xinetd" || return

	# Download the file.
	ncftpget -t 10 -r 2 -d $TCTMP/$TCID.out -u $temp_user -p password $SERVER \
		$TCTMP test_file.txt 1>$stdout 2>/dev/null
	tc_fail_if_bad "$?" "${errmsg[$?]}" "`cat $TCTMP/$TCID.out`" || return

	# See that it actually downloaded.
	[ -f $TCTMP/test_file.txt ]
	tc_fail_if_bad "$?" "Expected download file not found" || return

	# Does file have right contents?
	diff $home_dir/test_file.txt $TCTMP/test_file.txt >$stdout 2>$stderr
	tc_pass_or_fail "$?" "Downloaded file is corrupted"
}

####################################################################################
# MAIN
####################################################################################

TST_TOTAL=2

tc_setup

test_01 &&
test_02
