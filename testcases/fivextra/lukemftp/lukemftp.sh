#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003						      ##
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
# File :	lukemftp.sh
#
# Description:	Test the lukemftp package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Aug 11 2003 - created - RR
#		06 Jan 2004 - (RR) updated to tc_utils.source
#
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# global variables
#
SERVER="ftp3.linux.ibm.com"
REQUIRED="cat ls rm"

# for uploads NOT YET IMPLEMENTED
SERVER2="buell.ltc.austin.ibm.com"
UPFILE="lukemftp_testfile.txt"


################################################################################
# testcase functions
################################################################################

function lukemftp_01 {
	tc_register "Test that required lukemftp programs exist"
	tc_executes "lukemftp"
	tc_pass_or_fail $? "lukemftp package is not properly installed" || exit
}

function lukemftp_02 {
	tc_register "Test downloading a file"
	# -a == anonymous login
	lukemftp -a $SERVER:pub/MIRRORS.TXT 2>$stderr >/dev/null
	tc_fail_if_bad "$?" "lukemftp did not execute as expected." || return
	ls MIRRORS.TXT 1>/dev/null 2>$stderr
	tc_pass_or_fail "$?" "expected download file not found."
	rm -f MIRRORS.TXT &>/dev/null
}

function lukemftp_03 {
	tc_register "Test uploading a file"
	# -au == anonymous upload
	lukemftp -au ftp://$SERVER2/incoming/ $UPFILE 2>$stderr >$stdout
	tc_pass_or_fail "$?" "upload failed."
}

####################################################################################
# MAIN
####################################################################################

# Function:	main
#

#
# Returns:	- zero on success
#		- non-zero on failure
#
TST_TOTAL=2
tc_setup
tc_exec_or_break $REQUIRED || exit
lukemftp_01
lukemftp_02
#lukemftp_03

