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
# File :	apache.sh
#
# Description:	Check that apache can serve up an HTML web page.
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Mar 01 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		June15 2003 - add port - HelenPang
#		July 17 2003 - (rcp) use new fivget syntax.
#		Oct 16 2003 - use new local_setup and local_cleanup.
#			    - other code cleanup (rcp)
#		Nov 12 2003 (rcp) updated to tc_utils.source
#		14 Jan 2004 - (rcp) fixed check for required commands/files
#		09 Jun 2004 (rcp) fix logic for start/stop apache server
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

mypage=""
wasrunning="no"
startup="/etc/init.d/apache"
httpd_conf="/etc/httpd/httpd.conf"
docroot=""	# filled in by test01

#
# tc_local_setup
#
function tc_local_setup()
{
	tc_root_or_break || return
}

#
# tc_local_cleanup		cleanup unique to this testcase
#
function tc_local_cleanup()
{
	if [ "$wasrunning" = "no" ] ; then
		tc_info "stopping apache server"
		[ -e "$startup" ] && $startup stop >/dev/null
	fi
	[ -f $mypage ] && rm $mypage &>/dev/null
}

################################################################################
# the testcase functions
################################################################################

#
# test01	installation check
#
function test01()
{
	tc_register "installation check and start apache"

	# apache must be installed
	tc_executes $startup && tc_exists $httpd_conf
	tc_fail_if_bad $? "apache not installed properly" || return

	# docroot must exist
	docroot="`cat $httpd_conf | grep \"^DocumentRoot\"`"
	set $docroot
	local q="\""
	eval docroot=${2#\$q}		  # trim leading quote
	eval docroot=${docroot%\$q}       # trim trailing quote
	[ "$docroot" ] && tc_exists $docroot
	tc_fail_if_bad $? "DocumentRoot ($docroot) does not exist" || return

	# start server if not already running
	$startup status >$stdout 2>$stderr && wasrunning="yes"
	if [ "$wasrunning" != "yes" ] ; then
		tc_info "starting apache"
		$startup start >$stdout 2>$stderr
		tc_fail_if_bad $? "apache server would not start" || return
		sleep 2		# be sure server is ready to respond
	fi

	tc_pass_or_fail 0	# pass if we get this far

}

#
# test02	fetch web page
#
function test02()
{
	tc_register "get web page from server"
	tc_exec_or_break cat grep || return

	# Place web page in server's docroot.
	mypage="$docroot/$TCID$$.html"
	local expected="secret message $$!"
	cat > $mypage <<-EOF
		<html>
		<body>
		<p>
		$expected
		</p>
		</body>
		</html>
	EOF

	# get the page from apache sever via http
	local port=80
	fivget http localhost $port $TCID$$.html >$stdout 2>$stderr
	tc_fail_if_bad $? "failed to GET from server" || return

	# compare for expected content
	grep -q "$expected" $stdout 2>$stderr
	tc_pass_or_fail $? "" "expected to see: \"$expected\" in stdout" 
}

################################################################################
# main
################################################################################

TST_TOTAL=2

tc_setup

test01 &&
test02
