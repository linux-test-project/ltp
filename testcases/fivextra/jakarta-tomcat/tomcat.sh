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
# File :	tomcat.sh
#
# Description:	test tomcat in apache environment 
#		
#
# Author:	Helen Pang, hpang@us.ibm.com
#
# History:	June 12 2003 - Created. Helen Pang. hpang@us.ibm.com
#		June ?  2003 - Updates after code review.
#
#		16 Dec 2003 - (hpang) updated to tc_utils.source
################################################################################
# source the standard utility functions
###############################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

###############################################################################
#  global variables
###############################################################################
wasrunning="no"
startup="/etc/init.d/apache"
apache_conf="/etc/sysconfig/apache"

#
# mycleanup     cleanup unique to this testcase
#
function mycleanup()
{
	[ -s $startup ] && $startup stop &>/dev/null

	if [ -s $TCTMP/orig_apache_conf ] ; then
        mv -f $TCTMP/orig_apache_conf $apache_conf
        SuSEconfig --module apache &>/dev/null
        fi

        [ "$wasrunning" = "yes" ] && $startup start >&/dev/null
								
	cleanup
}	
################################################################################
# the testcase functions
################################################################################

#
# test01	check/start apache serve	        
#		
function test01()
{
        tc_register "check apache server"
	tc_exec_or_break cat grep || return

	# apache must be installed
	[ -s "$startup" ]
	tc_fail_if_bad $? "apache not installed" || return
	# httpd.conf file must exist
	local conf_file="/etc/httpd/httpd.conf"
	[ -s $conf_file ]
	tc_fail_if_bad $? "$conf_file does not exist" || return
	# docroot must exist
	docroot="`cat $conf_file | grep \"^DocumentRoot\"`"
	set $docroot
	local q="\""
	eval docroot=${2#\$q}             # trim leading quote
	eval docroot=${docroot%\$q}       # trim trailing quote
	[ "$docroot" ]
	tc_fail_if_bad $? "$doc_root does not exist" || return

	tc_info "taking seconds to allow apache server start"
	local result=$'\n'"`$startup restart 2>&1`"
	tc_fail_if_bad $? "apache server would not start" \
	                        "output:" "$result" || return
	
	tc_pass_or_fail 0 "Will Never Fail Here"
	
}

#
# test02	set/check MOD_TOMCAT on apache server	
#
function test02()
{
	tc_register "set and check MOD_TOMCAT"
	tc_exec_or_break cat cp echo grep || return

	# remember original status of apache
	$startup status | grep -q running && wasrunning="yes"
		 
	cp -a $apache_conf $TCTMP/orig_apache_conf
	tc_fail_if_bad $? "could not copy apache_file" || return

	echo HTTPD_SEC_MOD_TOMCAT="yes" >> $apache_conf

	SuSEconfig --module apache >$stdout 2>$stderr
	tc_fail_if_bad $? "could not configure apache" || return

	# to start the Tomcat deamon
	rctomcat start >$stdout 2>$stderr
	tc_fail_if_bad $? "could not start Tomcat server" || return

	$startup restart >$stdout 2>$stderr
	tc_fail_if_bad $? "could not start apache after configuring mod_tomcat" || return

	cat $stdout | grep -q Tomcat 2>$stderr
	tc_pass_or_fail $? "module Tomcat did not start" || return
}

#
# test03	 use GET method to get the "Hello" page from Servlet  
#
function test03()
{
	tc_register "use GET method to get the \"Hello\" page from Servlet"
	tc_exec_or_break cat grep || return
	
	local mypage="/examples/servlets/helloworld.html"
	local mycontent="Hello"
	local port=8080
	fivget http localhost $port $mypage >$stdout 2>$stderr
#	fivget localhost $port $mypage >$stdout 2>$stderr
	expected=$mycontent
	cat $stdout | grep "$expected" >/dev/null
	tc_pass_or_fail $? "" "expected to see: \"$expected\"" \
		"in the following:"$'\n'"$result"
}

#
# test04	use GET method to get the date page from JSP
#			  
function test04()
{
	tc_register "use GET method to get the date page from JSP"
	tc_exec_or_break echo grep || return

	local mypage="/examples/jsp/dates/date.jsp"
	local mycontent="Day of month:"
	local port=8080
#	fivget http localhost $port $mypage >$stdout 2>$stderr
	fivget localhost $port $mypage >$stdout 2>$stderr
	local expected=$mycontent
	cat $stdout | grep "$expected" >/dev/null
	tc_pass_or_fail $? "" "expected to see: \"$expected\"" \
	        "in the following:"$'\n'"$result"
				
}

################################################################################
# main
################################################################################

TST_TOTAL=4

# standard tc_setup
tc_setup

tc_root_or_break || exit

trap "mycleanup" 0

test01 &&
test02 &&
test03 &&
test04 

