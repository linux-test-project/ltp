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
# File :	xml4c.sh
#
# Description:	Test's XML4C version 1.0 and 2.0's APIs on SAX: 
#		SAXCount, SAX2Count, SAXPrint,
#		SAX2Print, StdInParse
#
# Author:	Helen Pang, hpang@us.ibm.com
#
# History:	July 30 2003 - Created. Helen Pang. hpang@us.ibm.com
#
#		16 Dec 2003 - (hpang) updated to tc_utils.source
################################################################################
# source the standard utility functions
###############################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

SAXout="(10 elems, 4 attrs, 0 spaces, 99 chars)"

function tc_local_setup()
{
	cat <<-EOF > $TCTMP/SAXPrint.out
	<?xml version="1.0" encoding="LATIN1"?>
	<department>
	 <employee id="J.D">
	  <name>John Doe</name> 
	  <email>John.Doe@foo.com</email> 
	 </employee>
	 <employee id="B.S">
	  <name>Bob Smith</name> 
	  <email>Bob.Smith@foo.com</email> 
	 </employee>
	 <employee id="A.M">
	  <name>Alice Miller</name> 
	  <url href="http://www.foo.com/~amiller/"></url> 
	 </employee>
	</department>
	EOF
}

################################################################################
# the testcase functions
################################################################################

#
# test01       test SAXCount
#
function test01()
{
        tc_register "test SAXCount"

	SAXCount department.xml 2>$stderr >$stdout
	grep -q "$SAXout" < $stdout
        tc_pass_or_fail $? "unexpected output" \
		"expected to see \"$SAXout\""

}

#
# test02	test SAX2Count	
#
function test02()
{
	tc_register "test SAX2Count"

	SAXCount department.xml 2>$stderr >$stdout
	grep -q "$SAXout" < $stdout 2>>$stderr
        tc_pass_or_fail $? "unexpected output" \
		"expected to see \"$SAXout\""
}

#
# test03	test SAXPrint	
#
function test03()
{
	tc_register "test SAXPrint"

	SAXPrint department.xml >$stdout
	diff -w $stdout $TCTMP/SAXPrint.out &>$stderr
	tc_pass_or_fail $? "expected to see" $'\n'"$(cat $TCTMP/SAXPrint.out)"

}

#
# test04	test SAX2Print
#
function test04()
{
	tc_register "test SAX2Print"

	SAX2Print department.xml >$stdout
	diff -w $stdout $TCTMP/SAXPrint.out &>$stderr
	tc_pass_or_fail $? "expected to see" $'\n'"$(cat $TCTMP/SAXPrint.out)"

}

#
# test05	test StdInParse
#
function test05()
{
	tc_register "test StdInParse"

	StdInParse < department.xml 2>$stderr >$stdout
	grep -q "$SAXout" < $stdout
        tc_pass_or_fail $? "unexpected output" \
		"expected to see \"$SAXout\""
}


################################################################################
# main
################################################################################

TST_TOTAL=5

# standard tc_setup
tc_setup

tc_root_or_break || exit
tc_exec_or_break "diff grep cat"

test01 &&\
test02 &&\
test03 &&\
test04 &&\
test05 
