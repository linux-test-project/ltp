#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab:
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
# File :	mcp-misc.sh
#
# Description:	Miscelaneous mcp-specific testcass, including bug-verification
#		tests.
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Feb 22 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		16 Dec 2003 - (rcp) updated to tc_utils.source
#               02 Feb 2004 - Robb Romans - add test for distroid.
#		22 Mar 2004 (rcp) display misc info.
#

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# the testcase functions
################################################################################

#
# tmp01	permissions on /tmp
#
function tmp01()
{
	tc_register "tmp01 - check permissions on /tmp"
	tc_exec_or_break ls grep || return
	local expected="drwxrwxrwt"
	local actual="`ls -ld /tmp 2>$stderr`"
	echo "$actual" | grep "^$expected" >/dev/null
	tc_pass_or_fail $? "Unexpected results" \
		"expected \"$expected\"" \
		"  actual \"$actual\""
}

#
# tmp02	ownership of /tmp
#
function tmp02()
{
	tc_register "tmp02 - check ownership of /tmp"
	tc_exec_or_break ls grep || return
	local expected="0 0"
	local actual="`ls -nld /tmp 2>$stderr`"
	echo "$actual" | grep "^.* 0 *0" >/dev/null
	tc_pass_or_fail $? "Unexpected results" \
		"expected \"$expected\"" \
		"  actual \"$actual\""
}

#
# manifest	existence of /etc/manifest file
#
function manifest()
{
	tc_register "/etc/jlbd.manifest"
	[ -s "/etc/jlbd.manifest" ]
	tc_pass_or_fail $? "does not exist or is empty"
}

#
# bug1782	Ensure CPAN modules removed from build
#
function bug1782()
{
	tc_register "BUG1782 - ensure CPAN files are removed from build"
	tc_exec_or_break find || return
	local result="`find /lib -name \"*CPAN*\"; \
			find /usr/lib -name \"*CPAN*\"`"
	[ -z "$result" ]
	tc_pass_or_fail $? "CPAN was found at"$'\n'"$result"
}

#
# distroid     Test for existence of distro id.
#
function distroid()
{
	tc_register "/proc/distroid"
	[ -r /proc/distro_id ]
	tc_fail_if_bad $? "Could not find /proc/distro_id" || return
	tc_exec_or_break cat || return
	cat /proc/distro_id > $stdout
	tc_exec_or_break grep || return
	grep -q mcp $stdout 2>$stderr
	tc_pass_or_fail $? "does not contain correct info." \
		"Actual contents are in stdout. Expected to see \"mcp\""
}

#
# disp-info	Display interesting info
#
function disp_info()
{
	tc_register "====================================================="
	tc_info "$TCNAME"
	tc_info "$(cat /etc/jlbd.manifest | grep _MANIFEST_MAGIC_TAG)"
	tc_executes rpm &&
		tc_info "LTP version:    $(rpm -qa | grep ltp)"
	tc_info "LTP build date: $(cat mcp-misc-build-date)"
	tc_pass_or_fail 0	# always passes
}

################################################################################
# main
################################################################################

TST_TOAL=6
tc_setup

disp_info
manifest
distroid
tmp01
tmp02
bug1782
