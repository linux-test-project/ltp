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
# File :	locate.sh
#
# Description:	Test the updatedb and locate commands
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Oct 30 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		03 Jan 2004 - (rcp) updated to tc_utils.source

################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

clean_core=/etc/cron.daily/clean_core
updatedb_cron=/etc/cron.daily/updatedb
locate=/usr/bin/locate
updatedb=/usr/bin/updatedb
config=/etc/sysconfig/locate
loocate_this=""			# set by tc_local_setup

################################################################################
# utility functions specific to this script
################################################################################

function tc_local_setup()
{
	tc_root_or_break || return
	tc_add_user_or_break || return
	locate_this=$temp_user_home/locate_this_$$	# file to locate
	touch $locate_this
}

function tc_local_cleanup()
{
	true #	[ -e "$locate_this" ] && rm $locate_this
}

################################################################################
# the testcase functions
################################################################################

#
# test01	check that locate is installed
#
function test01()
{
	tc_register	"installation check"
	tc_executes $clean_core $updatedb_cron $locate $updatedb
	tc_fail_if_bad $? "locate not installed" || return
	tc_exist_or_break $config
	tc_pass_or_fail $? "locate's config file not installed"
}

#
# test02	run updatedb via manual invocation of cron script
#
function test02()
{
	tc_register	"updatedb"

	tc_info "invoking \"$updatedb_cron\" ... "
	$updatedb_cron >$stdout 2>$stderr
	tc_pass_or_fail $? "unexpected response from $updatedb_cron"
}

#
# test03	locate a file
#
function test03()
{
	tc_register	"locate a file"

	local locate_me=${locate_this##.*/}
	$locate	$locate_me >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from \"$locate $locate_me\"" ||
		return

	grep -q "$locate_me" $stdout 2>$stderr
	tc_pass_or_fail $? "file \"$locate_me\" not located"
}

################################################################################
# main
################################################################################

TST_TOTAL=3

tc_setup

test01 &&
test02 &&
test03
