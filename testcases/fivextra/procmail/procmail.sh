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
# File :	procmail.sh
#
# Description:	Tests procmail's ability to be inviked via $HOME/.forward
#		and direct mail to specific mailbox.
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Mar 09 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		May 13 2003 - Added "tc_root_or_break" check, use of $stderr
#		16 Dec 2003 - (robert) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

home=""
message=""		# text to be in email placed here by tc_local_setup

################################################################################
# local utility function
################################################################################

#
# tc_local_setup
#
function tc_local_setup()
{
	tc_root_or_break || return

	tc_add_user_or_break || return
	tc_exec_or_break cat sed chown || return

	# get temp user's home directory
	home=`sed -n "/^\$temp_user:/p" /etc/passwd | cut -d':' -f 6`

	# create forwarding file for temp user
	cat > $home/.forward <<-EOF
		"|exec /usr/bin/procmail"
	EOF
	chown $temp_user $home/.forward

	# create procmailrc file for temp user
	cat > $home/.procmailrc <<-EOF
		:0
		* ^Subject:.*XXXX
		XXXX-mail
		:0
		* ^From.*$temp_user
		from_me
	EOF
	chown $temp_user $home/.procmailrc

	# create message to be mailed and filed by userid and subject line
	message="A message from me $$"
	cat > $home/mailmessage <<-EOF
		$message
	EOF
	chown $temp_user $home/mailmessage
	return 0
}

################################################################################
# the testcase functions
################################################################################

#
# test01	Test procmail's ability to be invoked via $HOME/.forward and
#		direct mail to a specific file based on sender's name.
#

function test01()
{
	tc_register "store mail by user"
	tc_exec_or_break cat chown su sleep || return

	# have temp user mail msg to self, subject not important
	local cmd1="mail $temp_user -s \"fiv message\" < mailmessage"
	2>$stderr echo "$cmd1" | su - $temp_user
	tc_fail_if_bad  $? "bad results from $cmd1" || return

	# have temp user mail msg to self, special subject with XXXX
	# this is for use in test02. done here so we only need one sleep.
	local cmd2="mail $temp_user -s \"fiv message XXXX\" < mailmessage"
	2>$stderr echo "$cmd2" | su - $temp_user
	tc_fail_if_bad  $? "bad results from $cmd2" || return

	# give mail forwarding some tiime to complete
	tc_info "give mail forwarding 6 sec to complete"
	sleep 6

	# check that file created with correct contents
	[ -s $home/from_me ]
	tc_fail_if_bad $? "forwarded mail did not reach $home/from_me" || return

	# check that from_me contents are correct
	local result="`cat $home/from_me`"
	cat $home/from_me > $stdout
	cat $stdout | grep "$message" >/dev/null
	tc_pass_or_fail $? "message not forwarded correctly" \
		"Expected to see \"$message\" in stdout"
}

#
# test02	Test procmail's ability to be invoked via $HOME/.forward and
#		direct mail to a specific file based on subject
#
function test02()
{
	tc_register "store mail by subject"
	tc_exec_or_break grep cat echo || return

	# check that file was created with correct contents
	[ -s $home/XXXX-mail ]
	tc_fail_if_bad $? "forwarded mail did not reach $home/XXXX-mail" || return

	# check that XXXX-mail contents are correct
	cat $home/XXXX-mail > $stdout
	cat $stdout | grep "$message" >/dev/null
	tc_pass_or_fail $? "message not forwarded correctly" \
		"Expected to see \"$message\" in stdout"
}

################################################################################
# main
################################################################################

TST_TOTAL=2

tc_setup

test01 &&
test02		# depends on mail sent in test01

