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
# File :	ncftp.sh
#
# Description:	Test the ncftp package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Jun 13 2003 - created - RR
#		Jul 14 2003 - modified per peer review - RR
#		Jul 17 2003 - fixed sourcing of utility functions so that
#				the testcase runs under PAN, not just out
#				of its own directory - RCP
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
REQUIRED="cat ls rm tail"
# ncftpbatch reads this environment variable:
export HOME=`pwd`

# for uploads NOT YET IMPLEMENTED
SERVER2=""
UPFILE="ncftp_testfile.txt"

# array to parse error codes into readable string
errmsg[1]="ERROR: Could not connect to remote host."
errmsg[2]="ERROR: Could not connect to remote host - timed out."
errmsg[3]="ERROR: Transfer failed."
errmsg[4]="ERROR: Transfer failed - timed out."
errmsg[5]="ERROR: Directory change failed."
errmsg[6]="ERROR: Directory change failed - timed out."
errmsg[7]="ERROR: Malformed URL."
errmsg[8]="ERROR: Usage error."
errmsg[9]="ERROR: Error in login configuration file."
errmsg[10]="ERROR: Library initialization failed."
errmsg[11]="ERROR: Session initialization failed."

################################################################################
# testcase functions
################################################################################

function ncftp_00 {
    tc_register "Test that required ncftp programs exist"
    tc_executes ncftp ncftpls ncftpget ncftpput ncftpbatch ncftpspooler
    tc_pass_or_fail $? "ncftp package is not properly installed" || return
}

function ncftp_01 {
	tc_register "Test downloading a file"
	cat test_01.txt | ncftp $SERVER &>/dev/null
	ls MIRRORS.TXT 1>/dev/null 2>$stderr
	tc_pass_or_fail "$?" "expected download file not found."
	rm -f MIRRORS.TXT &>/dev/null
}

function ncftp_02 {
	tc_register "Test NcFTPls"
	# -d writed debugging information
	ncftpls -d $TCTMP/ncftp_02.out ftp://$SERVER/pub/ &>/dev/null
	tc_pass_or_fail "$?" "Failed: ${errmsg[$?]}" "`tail -20 $TCTMP/ncftp_02.out`"
}

function ncftp_03 {
	tc_register "Test NcFTPGet"
	# -d writes debugging information
	ncftpget -d $TCTMP/ncftp_03.out $SERVER $TCTMP /pub/NOTICE.TXT 2>/dev/null
	tc_fail_if_bad "$?" ${errmsg[$?]} || return
	ls $TCTMP/NOTICE.TXT 1>/dev/null 2>$stderr
	tc_pass_or_fail "$?" "expected download file not found" "`tail -20 $TCTMP/ncftp_03.out`"
	rm -f $TCTMP/NOTICE.TXT $TCTMP/ncftp_03.out &>/dev/null
}

function ncftp_04 {
	tc_register "Test NcFTPPut"
	# -d writed debugging information
	ncftpput -d $TCTMP/ncftp_04.out ftp://$SERVER2/incoming/ $TCTMP/$UPFILE &>/dev/null
	tc_pass_or_fail "$?" "Failed: ${errmsg[$?]}" "`tail -20 $TCTMP/ncftp_04.out`"
}

function ncftp_05 {
	tc_register "Test NcFTPbatch"
	# Create a job queue
	ncftpget -bb $SERVER $TCTMP /pub/NOTICE.TXT 2>/dev/null
	tc_fail_if_bad "$?" "Unable to create the job queue: ${errmsg[$?]}"
	# process the job queue
	ncftpbatch -D &>/dev/null
	tc_fail_if_bad "$?" "Unable to process the job queue: ${errmsg[$?]}" || return
	ls $TCTMP/NOTICE.TXT >/dev/null 2>$stderr
	tc_pass_or_fail "$?" "expected download file not found"
	rm -f $TCTMP/NOTICE.TXT &>/dev/null
}

function ncftp_06 {
	# this testcase requires root priveleges.
	# spooler must access global dir /var/spool/ncftp
	tc_root_or_break || return
	tc_register "Test NcFTPspooler"
	killall ncftpspooler &>/dev/null
	# Create a job queue
	ncftpget -bb $SERVER $TCTMP /pub/NOTICE.TXT 2>/dev/null
	tc_fail_if_bad "$?" "Unable to create the job queue: ${errmsg[$?]}" || return
	# Process the queue
	ncftpspooler -d
	tc_pass_or_fail "$?" "Unable to process the job queue: ${errmsg[$?]}"
	rm -f $TCTMP/NOTICE.TXT &>/dev/null
	killall ncftpspooler &>/dev/null
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
tc_setup
tc_exec_or_break $REQUIRED || exit
ncftp_00 &&
ncftp_01 &&
ncftp_02 &&
ncftp_03 &&
# ncftp_04 &&
ncftp_05 &&
ncftp_06
