#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003                                               ##
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
# File :	fetchmail.sh
#
# Description:	Test fetchmail package
#
# Author:	Robb Romans <robb@austin.ibm.com>
#
# History:	Aug 19 2003 - created - RR
#               Portions borrowed from mail_tests.sh
#		Sep 23 2003 - RCP - modified to use tc_add_user_or_break from
#					tc_utils.source
#		06 Jan 2004 - updated to tc_utils.source - RR
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# Variable definitions
################################################################################

REQUIRED="cat grep mail ping"
MAILSERVER="buell.ltc.austin.ibm.com"
REM_USER="fivtest"

################################################################################
# utility functions local to this testcase
################################################################################

function tc_local_setup
{
	tc_root_or_break || exit
	tc_exec_or_break $REQUIRED || return

	# check if mailserver is up
	ping -c 1 -w 10 $MAILSERVER &>/dev/null
	tc_break_if_bad	"$?" "$MAILSERVER is unreachable" || exit

	# check if exists / add the user test on this system.
	if ! grep "^$REM_USER:" /etc/passwd ; then
		tc_add_user_or_break $REM_USER || exit
	fi

	# remove any old mail
	tc_info "Removing existing mail for user $REM_USER."
	rm -f /var/spool/mail/$REM_USER &>/dev/null
}

################################################################################
# testcase functions
################################################################################

function test_inst {
	tc_register "Is fetchmail installed?"
	tc_executes fetchmail
	tc_pass_or_fail "$?" "fetchmail is not properly installed"
}

function fetch_mail {

	tc_register "Fetch mail from $MAILSERVER"

	# send the mail to be fetched
	tc_info "Sending a mail to $REM_USER@$MAILSERVER."
	cat > $TCTMP/tst_mail.in <<-EOF
		This is a test email for fetchmail testing.
	EOF
	mail -s "FIVTEST" $REM_USER@$MAILSERVER < $TCTMP/tst_mail.in \
		1>$stdout 2>$stderr
	tc_fail_if_bad "$?" "FATAL: Unable to send mail to $MAILSERVER." || return

	# fetch the sent mail
	tc_info "Sleeping 30s to allow remote delivery."
	sleep 30
	fetchmail -f fetchmailrc 1>$stdout 2>$stderr
	tc_pass_or_fail "$?" "fetch_mail: fetchmail command failed."
}

function mail_test {

	tc_register "Test that fetched mail exists."
	tc_info "Sleeping 30s to allow local delivery."
	sleep 30
	local result=""
	[ -f /var/spool/mail/$REM_USER ]
	tc_fail_if_bad "$?" "No mail file written for $REM_USER" || return

	result=$(grep 'FIVTEST' /var/spool/mail/$REM_USER)
	
	[ ! -z "$result" ]
	tc_pass_or_fail "$?" "Mail sent to $REM_USER, but was not received"

}

################################################################################
# MAIN
################################################################################

# Function:	main
#
# Exit:		- zero on success
#		- non-zero on failure
#

TST_TOTAL=3
tc_setup

test_inst &&
fetch_mail &&
mail_test
