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
# File :	pkgcheck.sh
#
# Description:	Check the integrity of the package named in $1
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	02 Mar 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		04 Mar 2003 - Allow skip-list for files that may legitimatley
#			differ from original rpm install.
#		31 Oct 2003 (Boo!) RC Paulsen: Better verification check
#		03 Jan 2004 - (rcp) updated to tc_utils.source
#		07 Jan 2004 - (rcp) remove dependency check since our MCP
#				rpm's are bad at this.

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

PKGNAME=$1
PACKAGES="$@"

#
# tc_local_setup
#
function tc_local_setup()
{
	[ "$PKGNAME" ]
	tc_break_if_bad $? "Must supply name of package on command line" \
		|| return
	#tc_root_or_break || return
}

#
# skip		See if file ($1) is in file $PKGNAME.skip or in pkgcheck.skip.
#		If so, return true so caller can ignore bad result from
#		rpm -V $PKGNAME.
#
function skip()
{
	touch $TCTMP/pkgcheck.skip $TCTMP/$PKGNAME.skip
	[ -e "$LTPBIN/pkgcheck.skip" ] && cp $LTPBIN/pkgcheck.skip $TCTMP/
	[ -e "$LTPBIN/$PKGNAME.skip" ] && cp $LTPBIN/$PKGNAME.skip $TCTMP/
	echo $1 | grep -q -f $TCTMP/pkgcheck.skip -f $TCTMP/$PKGNAME.skip &&
		return 0
	return 1
}

################################################################################
# the testcase functions
################################################################################

#
# test01	check that PKGNAME is installed 
#
function test01()
{
	PKGNAME=$1
	tc_register "$PKGNAME installed?"
	tc_exec_or_break rpm || return
	rpm -q $PKGNAME 2>$stderr >$stdout
	tc_pass_or_fail $? "$PKGNAME is not installed"
}

#
# test02	use rpm -V to verify files
#
function test02()
{
	PKGNAME=$1
	tc_register "rpm -V $PKGNAME"
	tc_exec_or_break rpm grep || return

	local fail=no
	rpm -V $PKGNAME 2>$stderr > $TCTMP/$TCID.err
	while read line ; do
		fail=no
		set $line

		# skip file if in skip list
		local file=$2			# filenmame may be 2nd or 3rd
		[ "$3" ] && file=$3		# item on the line
		skip $file && continue

		# fail if M, D, L, U, or G
		# removed: echo $line | grep -qi "^Unsatisfied depend" ||
		if 
			echo $line | grep -q "^.M......" ||
			echo $line | grep -q "^...D...." ||
			echo $line | grep -q "^....L..." ||
			echo $line | grep -q "^.....U.." ||
			echo $line | grep -q "^......G." ||
			echo $line | grep -qi "^missing" ; then
				fail=yes
		fi
		[ "$fail" = "yes" ] && tc_info "$line"
	done < $TCTMP/$TCID.err

	[ "$fail" = "no" ]
	tc_pass_or_fail $? "above files did not verify"
}

################################################################################
# main
################################################################################


tc_setup			# standard setup

TST_TOTAL=0
while [ -n "$1" ] ; do
	let TST_TOTAL+=2
	test01 $1 &&
	test02 $1
	shift
done
