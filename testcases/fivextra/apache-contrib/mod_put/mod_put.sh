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
# File :	mod_put.sh
#
# Description:	Implement mod_put's PUT/DELETE methods on apache
#
# Author:	Helen Pang, hpang@us.ibm.com
#
# History:	June 2 2003 - Created. Helen Pang, hpang@us.ibm.com
# Updated:	June ? 2003 - hpang@us.ibm.com
#		July 17 2003 - rcp: 
#			     1. Use new fivget syntax
#			     2. test04 should look for mycontent, not mypage
#		Aug 26 2003 - rcp: restore httpd_conf file
#		Oct 16 2003 - use new tc_local_setup and tc_local_cleanup.
#			    - other code cleanup (rcp)
#		Nov 11 2003 (rcp) updated to tc_utils.source
#		Jan 14 2004 (rcp) fixed installation check
###############################################################################
# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################
wasrunning="no"
startup="/etc/init.d/apache"
apache_conf="/etc/sysconfig/apache"
httpd_conf="/etc/httpd/httpd.conf"
testdir="tester$$"
wwwowner=wwwrun
docroot=""	# filled in by test01

#
# tc_local_setup
#
function tc_local_setup()
{
	tc_root_or_break || return
	tc_exec_or_break SuSEconfig || return
}

#
# tc_local_cleanup
#
function tc_local_cleanup()
{
	[ -x $startup ] && $startup stop &>/dev/null

        tc_info  "remove testdir under server's docroot"
	[ -d "$docroot" ] && rm -fr $docroot/$testdir

	[ -f $TCTMP/orig_httpd_conf ] &&
		mv -f $TCTMP/orig_httpd_conf $httpd_conf

	[ -f $TCTMP/orig_apache_conf ] &&
		mv -f $TCTMP/orig_apache_conf $apache_conf

	tc_executes SuSEconfig && SuSEconfig --module apache &>/dev/null

        [ "$wasrunning" = "yes" ] && $startup start >&/dev/null
}

################################################################################
# the testcase functions
################################################################################

#
# test01	installation check
#
function test01()
{
	tc_register "apache installation check"

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
}

#
# test02	start apache with mod_put
#	
function test02()
{
	tc_register "start apache with mod_put"
	tc_exec_or_break cp echo grep || return
	
	# remember original status of apache
	$startup status | grep -q running && wasrunning="yes"

	# configure mod_put
	cp -a $apache_conf $TCTMP/orig_apache_conf
	echo HTTPD_SEC_MOD_PUT="yes" >> $apache_conf
	SuSEconfig --module apache >$stdout 2>$stderr
	tc_fail_if_bad $? "could not configure apache" || return
	
	# restart apache with new config
	$startup restart >$stdout 2>$stderr
	tc_fail_if_bad $? "could not start apache after configuring mods" || return

	# ensure module loaded
	grep -q put $stdout 2>$stderr
	tc_pass_or_fail $? "module put did not start"
	sleep 2		# be sure server is ready to respond
}	

#
# test03	PUT file to http server
#
function test03()
{
	tc_register "PUT file to http server"
	tc_exec_or_break grep || return

	mkdir -p $docroot/$testdir
	chown $wwwowner $docroot/$testdir

	cp -a $httpd_conf $TCTMP/orig_httpd_conf
	echo "Alias /$testdir/ "$docroot/$testdir/"" >>$httpd_conf
	echo "<Directory "$docroot/$testdir">" >>$httpd_conf
	echo "EnablePut On" >>$httpd_conf
	echo "EnableDelete On" >>$httpd_conf
	echo "</Directory>" >>$httpd_conf

	$startup restart >/dev/null
	mypage=/$testdir/mod_put$$.html
	mycontent="<html><body><p>secret_message_$$</p></body></html>"
	fivput localhost $mypage \"$mycontent\" >$stdout 2>$stderr
	expected="$mypage created"
	grep -q "$expected" $stdout 2>$stderr
        tc_pass_or_fail $? "" "expected to see: \"$expected\" in stdout"; 
}

#
# test04	GET the uploaded file
#
function test04()
{
	tc_register "GET the uploaded file"
	tc_exec_or_break grep || return
	
	port=80
	fivget http localhost $port $mypage >$stdout 2>$stderr
	expected=$mycontent
	grep -q "$expected" $stdout 2>>$stderr
	tc_pass_or_fail $? "" "expected to see: \"$expected\"" \
	        "in the following:"$'\n'"$result"
}

#
# test05	DELETE the upload html file
#
function test05()
{
	tc_register "DELETE the upload html file"
	tc_exec_or_break grep || return
	
	fivdelete localhost $mypage >$stdout 2>$stderr
	expected="$mypage deleted"
	grep -q "$expected" $stdout 2>>$stderr
        tc_pass_or_fail $? "" "expected to see: \"$expected\"" \
	        "in the following:"$'\n'"$result"
}

#
# test06	GET of deleted file should fail
#
function test06()
{
	tc_register "GET of deleted file should fail"
	tc_exec_or_break grep || return
	
	fivget http localhost $port $mypage >$stdout 2>$stderr
	expected="$mypage was not found"
	grep -q "$expected" $stdout 2>>$stderr
        tc_pass_or_fail $? "" "expected to see: \"$expected\"" \
	        "in the following:"$'\n'"$result"
}


################################################################################
# main
################################################################################

TST_TOTAL=6

tc_setup

test01 &&
test02 &&
test03 &&
test04 &&
test05 &&
test06

