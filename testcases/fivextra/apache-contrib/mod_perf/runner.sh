#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2001		      ##
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
# File :	runner.sh
#
# Description:	Check that apache can serve up an HTML web page.
#
# Author:	Helen Pang, hpangen@us.ibm.com
#
# History:	June 4, 2003 - Created. Helen Pang, hpangen@us.ibm.com
#
#
#		Nov 11 2003 (rcp) updated to tc_utils.source

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

function runtest()
{
	tc_register "get web page $repeat times"
	myfname="$testdir/$TCID$$.html"
	port=80

	mypage="$docroot/$myfname"
	local expected="secret message $$!"
	cat > $mypage <<-EOF
		<html> <body> <p> $expected </p> </body> </html>
	EOF

	# get the page from apache sever via http
	count=1
	while [ "$count" -le $repeat ]
	do
		fivget http $host $port $myfname >$stdout 2>$stderr
		cat $stdout | grep -q "$expected" 2>>$stderr
		tc_fail_if_bad $? "didnt see $expected in stdout" || return
		let count+=1
	done
	tc_pass_or_fail 0 "never fails here"
}

TST_TOTAL=1

tc_setup

host=$1
docroot=$2
repeat=$3
testdir=$4

runtest
