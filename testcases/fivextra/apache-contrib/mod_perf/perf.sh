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
# File :	perf.sh
#
# Description:	Test mod_bandwidth and mod_throttle applied on apache by driving
#		generate parallel workload 
#
# Author:	Helen Pang, hpang@us.ibm.com
#
# History:	June 2 2003 - Created. Helen Pang, hpang@us.ibm.com
# Updated:	June ? 2003 - hpang@us.ibm.com
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

mypage=""
wasrunning="no"
startup="/etc/init.d/apache"
apache_conf="/etc/sysconfig/apache"
httpd_conf="/etc/httpd/httpd.conf"
testdir="tester$$"
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
# test02	start apache with mod_bandwidth and mod_throttle
#
function test02()
{
	tc_register "start apache with mod_bandwidth and mod_throttle"
	tc_exec_or_break grep || return

	# remember original status of apache
	$startup status | grep -q running && wasrunning="yes";

	# configure mod_bandwidth and mod_throttle
	cp -a $apache_conf $TCTMP/orig_apache_conf
	echo HTTPD_SEC_MOD_BANDWIDTH="yes" >> $apache_conf
	echo HTTPD_SEC_MOD_THROTTLE="yes" >> $apache_conf
	SuSEconfig --module apache >$stdout 2>$stderr
	tc_fail_if_bad $? "could not configure apache" || return

	# restart apache with new config
	$startup restart >$stdout 2>$stderr
	tc_fail_if_bad $? "could not start apache after configuring mods" || return

	# ensure modules loaded
	grep bandwidth $stdout | grep -q throttle 2>$stderr
	tc_pass_or_fail $? "modules bandwidth, throttle did not start"
	sleep 2		# be sure server is ready to respond
}

#
# test03	multi-access apache server via http
#
function test03()
{
	tc_register "mult-access apache via http"
	tc_exec_or_break mkdir || return
	
	mkdir -p $docroot/$testdir

	local tid=1
	local num_proc=30
	local num_rep=10
	while [ "$tid" -le $num_proc ]
	do
		let tid+=1
		./runner.sh localhost $docroot $num_rep $testdir &
	done
	wait

	tc_pass_or_fail $? "error fetching web pages"
}

################################################################################
# main
################################################################################

TST_TOTAL=3

tc_setup

test01 &&
test02 &&
test03
