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
# File :	goahead-webserver.sh
#
# Description:	Check that goahead-webserver can serve up an HTML web page.
#
# Author:
#
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
wasinstalled="no"
checkport80="FAIL"
#startup="/opt/%{HOST}/bin/webs"
#docroot="/opt/%{HOST}/web"
#binpath="/opt/%{HOST}/bin"

startup="/usr/bin/webs"
docroot="/var/web"
binpath="/usr/bin"

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
   local pid
   local i
        if [ "$wasinstalled" = "no" ] ; then
	       
	      return 
	fi

        if [ "$checkport80" = "FAIL" ] ; then
	     
	      return
	fi      

	if [ "$wasrunning" = "no" ] ; then
		 tc_info "stopping web server"
		 pid=` ps -aux |grep "/webs$" |awk '{ print $2; }' `

                 for i in $pid; do
                 kill -TERM $i >/dev/null 2>&1
                 done

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
   local pid
   local i

	tc_register "installation check and start webserver"
	tc_exec_or_break awk grep ps netstat wc|| return

	# goahead-webserver must be installed
	tc_executes $startup
	tc_fail_if_bad $? "goahead-webserver not installed properly" || return

        wasinstalled="yes"

        # check goahead-webserver status

        pid=` ps -aux |grep "/webs$" |awk '{ print $2; }' `
   
        for i in $pid; do
        wasrunning="yes"
        done

        # check the port 80

        if [ "$wasrunning" = "no" ] ; then
	        local num=`netstat -ant|grep :80|grep LISTEN |wc -l` 
	        tc_break_if_bad $num "port 80 was used by other program." || return 

                checkport80="PASS"

	# start goahead-webserver

	        cd $binpath
	        ./webs &>/dev/null 2>&1 & 
	fi

		tc_pass_or_fail 0 ""
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

        sleep 1 # let web server start properly  
	
	# get the page from web sever via http
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
