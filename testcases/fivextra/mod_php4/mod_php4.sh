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
# File :	mod_php4.sh
#
# Description:	Check that apache can serve up a PHP web page.
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Mar 01 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		June 15 2003 - add port - HelenPang
#		July 17 2003 - use new fivget syntax (rcp)
#		Oct 16 2003 - use new tc_local_setup and tc_local_cleanup.
#			    - other code cleanup (rcp)
#		02 Dec 2003 (rcp) updated to tc_utils.source
#			- added snmp_test. Invoke with -s to include this test.
#				This has not yet been tested.
###############################################################################
# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

mypage=""
do_snmp="no"
we_started_apache=""
we_started_snmp=""
apacheinit="/etc/init.d/apache"
snmpinit="/etc/init.d/snmpd"
httpd_conf="/etc/httpd/httpd.conf"

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

	if [ "$we_started_apache" = "yes" ] ; then
		[ -x "$apacheinit" ] && { 
			tc_info "stopping apache server"
			$apacheinit stop >/dev/null
		}
	fi
	[ -f $mypage ] && rm $mypage &>/dev/null

	if [ "$we_started_snmp" = "yes" ] ; then
		[ -x "$snmpinit" ] && { 
			tc_info "stopping snmp server"
			$snmpinit stop >/dev/null
		}
	fi
}

#
# usage
#
function usage()
{
	1>&2 echo "USAGE: $0 [-s]"
	1>&2 echo $'\t'"where -s optionally asks to use snmp php functions"
	exit 1
}

#
# parse command line arguments
# (so far, only one)
#
function parse_args()
{
	while getopts s opt ; do
		case "$opt" in
			s)	do_snmp="yes"
				let TST_TOTAL+=1
				;;
			*)	usage
				;;
		esac
	done
}

################################################################################
# the testcase functions
################################################################################

#
# test01	installation check
#
function test01()
{
	tc_register "installation check"

	# apache must be installed
	tc_executes $apacheinit
	tc_fail_if_bad $? "not properly installed" || return

	# httpd.conf file must exist
	tc_exist_or_break $httpd_conf
	tc_fail_if_bad $? "not properly installed" || return

	# DocumentRoot must be specified in httpd.conf
	docroot="`grep \"^DocumentRoot\" $httpd_conf`"
	set $docroot
	local q="\""
	eval docroot=${2#\$q}		# trim leading quote
	eval docroot=${docroot%\$q}	# trim trailing quote
	[ "$docroot" ]
	tc_pass_or_fail $? "DocumentRoot not specified in $httpd_conf"
}

#
# test02	fetch php page
#
function test02()
{
	tc_register "get PHP web page from server"
	tc_exec_or_break cat grep echo || return

	# Place web page in server's docroot.
	# Construct php web page in a way that the returned value will only
	# match the expected if the page has been procesed by PHP.
	mypage="$docroot/$TCID$$.php"
	local a="$TCID";
	local b="$$"
	local expected="$a $b"
	cat > $mypage <<-EOF
		<?php
			\$a="$a";
			\$b="$b";
			print "\$a \$b
			";
		?>
	EOF

	# start apache if not already running
	$apacheinit status >$stdout 2>$stderr || {
		tc_info	"starting apache web server"
		$apacheinit start >$stdout 2>$stderr
		tc_fail_if_bad $? "apache server would not start" || return
		sleep 2		# be sure server is ready to respond
		we_started_apache="yes"
	}

	# get the page from apache sever via http
	local port=80
	fivget http localhost $port $TCID$$.php >$stdout 2>$stderr
	tc_fail_if_bad $? "failed to GET from server" || return

	# compare for expected content
	grep -q "$expected" $stdout 2>$stderr
	tc_pass_or_fail $? "" "expected to see \"$expected\" in stdout" 
}


#
# snmp_test	fetch php page with snmp functions
#		only runs if -s command line switch is used.
#
function snmp_test()
{
	tc_register "PHP snmp functions"
	tc_exec_or_break cat grep echo || return

	tc_executes $snmpinit
	tc_fail_if_bad $? \
		"snmp test requested, but snmp not properly installed" || return

	# start snmpd if not already running
	$snmpinit status | grep -q running || {
		tc_info	"starting snmp server"
		$snmpinit start >$stdout 2>$stderr
		tc_fail_if_bad $? "snmp server would not start" || return
		sleep 2		# be sure server is ready to respond
		$snmpinit status | grep -q running
		tc_break_if_bad $? "could not start snmp server" || return
		we_started_snmp="yes"
	}

	# Place web page in server's docroot.
	# TODO: THis needs to be tested and expanded. Might need to do some
	# local snmp setup before this works.
	mypage="$docroot/$TCID$$.php"
	local a="$TCID";
	local b="$$"
	cat > $mypage <<-EOF
		<?php
			\$host = 'localhost';
			\$community = 'public';
			\$sysName = snmpget(\$host, \$community, "system.sysName.0");
			print "\$sysName
			";
			\$a = snmpwalk(\$host, \$community, "");
			for (\$i = 0; \$i < count(\$a); \$i++) {
				echo "\$a[\$i]
				";
			}
		?>
	EOF

	# get the page from apache sever via http
	local port=80
	fivget http localhost $port $TCID$$.php >$stdout 2>$stderr
	tc_fail_if_bad $? "failed to GET from server" || return

	# compare for expected content
	local exp1="$HOSTNAME"
	local exp2="$(uname -n)"
	local exp3="$(uname -m)"
	local exp4="$(uname -r)"
	local exp5="$(uname -s)"
	local exp6="$(uname -v)"
	local exp7="$0"

	grep -q "$exp1" $stdout 2>$stderr
	tc_fail_if_bad $? "expected to see \"$exp1\" in stdout"  || return

	grep -q "$exp2" $stdout 2>$stderr
	tc_fail_if_bad $? "expected to see \"$exp2\" in stdout"  || return

	grep -q "$exp3" $stdout 2>$stderr
	tc_fail_if_bad $? "expected to see \"$exp3\" in stdout"  || return

	grep -q "$exp4" $stdout 2>$stderr
	tc_fail_if_bad $? "expected to see \"$exp4\" in stdout"  || return

	grep -q "$exp5" $stdout 2>$stderr
	tc_fail_if_bad $? "expected to see \"$exp5\" in stdout"  || return

	grep -q "$exp6" $stdout 2>$stderr
	tc_fail_if_bad $? "expected to see \"$exp6\" in stdout"  || return

	grep -q "$exp7" $stdout 2>$stderr
	tc_fail_if_bad $? "expected to see \"$exp7\" in stdout"  || return

	tc_pass_or_fail 0 "always pass if we get this far"

}

################################################################################
# main
################################################################################

TST_TOTAL=2

tc_setup

parse_args "$@"

test01 &&
test02 &&
[ "$do_snmp" = "yes" ] && snmp_test

