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
# File :	mod_ssl.sh
#
# Description:	Check that apache can serve up an https web page.
#
# Author:	Robb Romans <robb@austin.ibm.com>
#		based on apache.sh by Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Apr 18 2003 - Created RR.
#		Apr 21 2003 - implement RCP's suggested changes.
#		June 15 2003 - add port and ssl option (-s flag) -HelenPang
#		July 17 2003 - use new fivget syntax (rcp)
#			     - use --module apache on cleanup's SuSEconfig.
#		15 Dec 2003 (rcp) updated to tc_utils.source
#				use new local_cleanup scheme
#				break into 3 testcases
#				remove some extraneous failure checks
#
# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################
mypage=""
apache_was_running="unknown"
we_started_apache="no"
we_configured_apache="no"
startup="/etc/init.d/apache"
conf_file="/etc/httpd/httpd.conf"
sysconfig_file="/etc/sysconfig/apache"

#
# tc_local_cleanup	cleanup unique to this script
#
function tc_local_cleanup()
{
	if [ "$we_started_apache" = "yes" ] ; then
		tc_info "stopping apache server"
		$startup stop >/dev/null
	fi
	if [ "$we_configured_apache" = "yes" ] ; then
		tc_info "restoring apache configuration" 
		[ -f $mypage ] && rm $mypage &>/dev/null
		[ -f $sysconfig_file-orig ] &&
			mv $sysconfig_file-orig $sysconfig_file
		[ -e /sbin/SuSEconfig ] &&
			/sbin/SuSEconfig --module apache &>/dev/null
	fi
	if [ "$apache_was_running" = "yes" ] ; then 
		tc_info "restarting apache server"
		$startup start >/dev/null
	fi
}

################################################################################
# the testcase functions
################################################################################

#
# installation check
#
function test01()
{
	tc_register "installation check"
	tc_executes $startup && tc_exists $conf_file $sysconfig_file
	tc_pass_or_fail $? "apache not properly installed"
}

#
# configure apache for mod_ssl
#
function test02()
{
	tc_register "configure apache for mod_ssl"
	tc_exec_or_break cat grep echo || return

	# generate certificate files
	$LTPBIN/mkcert.sh make --no-print-directory /usr/bin/openssl /usr/sbin/ dummy >$stdout 2>$stderr
	tc_fail_if_bad $? "failed to generate certificate files." || return
	tc_info "generated ssl certificate files"

	# configure apache for mod_ssl
	we_configured_apache="yes"
	cp $sysconfig_file $sysconfig_file-orig
	echo "HTTPD_SEC_MOD_SSL=yes" >>$sysconfig_file
	/sbin/SuSEconfig --module apache &>/dev/null
	tc_fail_if_bad $? "SuSEconfig failed to configure mod_ssl." || return

	# (re)start server
	$startup status >$stdout 2>$stderr &&
		apache_was_running="yes" &&
		action=restart || action=start
	we_started_apache="yes"
	tc_info "$action apache server"
	$startup $action >$stdout 2>$stderr
	tc_pass_or_fail $? "apache server would not start" || return
	sleep 2		# be sure server is ready to respond
}

#
# mod_ssl functionality
#
function test03()
{
	tc_register "fetch web page via https"

	# docroot must exist
	local docroot="`cat $conf_file | grep \"^DocumentRoot\"`"
	set $docroot
	local q="\""
	eval docroot=${2#\$q}		# trim leading quote
	eval docroot=${docroot%\$q#}	# trim trailing quote
	[ "$docroot" ]
	tc_fail_if_bad $? "$docroot does not exist" || return

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
	tc_info "placed HTTP web page on server"
	# get the page from apache sever via http
	local port=80
	fivget https localhost $port $TCID$$.html >$stdout 2>$stderr
	tc_fail_if_bad $? "failed to GET from server" || return

	# compare for expected content
	cat $stdout | grep -q "$expected"
	tc_pass_or_fail $? "" "expected to see: \"$expected\" in stdout" 
}

################################################################################
# main
################################################################################

TST_TOTAL=3
tc_setup		# standard tc_setup

tc_root_or_break || exit

test01 &&
test02 &&
test03
