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
# File :	dic.sh
#
# Description:	This testcase checks 'dtoa', 'atod' command of Free Wnn.
#
# Author:	Shoji Sugiyama (shoji@jp.ibm.com)
#
# History:	Aug 12 2003 - Initial version created. 
#		Aug 28 2003 - RCP: removed extraneous cleanup function.
#		05 Jan 2004 - (rcp) updated to tc_utils.source

################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

FWNNDIR=/var/lib/wnn

################################################################################
# any utility functions specific to this file can go here
################################################################################

################################################################################
# the testcase functions
################################################################################

#
# test01	
#
function test01()
{
	#
	# Register test case
	#
	tc_register "\"dtoa\" convert binary dictionary into text format."

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo find dtoa || return
	
	#
	# Execute and check result
	#
	for bindic in `find $FWNNDIR -name "*.dic" -print`
	do
		for opt in "-n" "-s" "-e" "-E" ""
		do
			# echo $bindic
			bname=`basename $bindic`
			# echo ${bname%.dic}.u
			dtoa $opt $bindic > $TCTMP/${bname%.dic}.u
			tc_fail_if_bad $? "[dtoa] fails to convert [$bindic] with option [$opt]" || return
		done
	done	

	tc_pass_or_fail 0 "PASSed if we get this far"
}

#
# test02	
#
function test02()
{
	#
	# Register test case
	#
	tc_register "\"atod\" convert text dictionary into binary format."

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo find atod || return
	
	#
	# Execute and check result
	#
	for textdic in `ls $TCTMP/*.u`
	do
		# for opt in "-s 100000" "-R" "-S" "-U" "-r" "-N" "-n" "-l"
		for opt in "" "-R" "-S" "-r" "-N" "-n" "-I"
		do
			# echo $textdic
			bname=`basename $textdic`
			# echo ${bname%.u}.dic
			atod $opt $TCTMP/${bname%.u}.dic < $textdic > /dev/null 2>&1
			tc_fail_if_bad $? "[atod] fails to convert [$textdic] with option [$opt]" || return
		done
	done	
	tc_pass_or_fail 0 "PASSed if we get this far"
}

#
# test03	
#
function test03()
{
	#
	# Register test case
	#
	tc_register "\"wnntouch\" update dictionary header information."

	#
	# Check if prereq commands are existed.
	#
	tc_exec_or_break echo find wnntouch || return
	
	#
	# Execute and check result
	#
	for bindic in `ls $TCTMP/*.dic`
	do
		wnntouch $bindic > $stdout 2> $stderr
		tc_fail_if_bad $? "[wnntouch] fails to update [$textdic]" || return
	done	
	tc_pass_or_fail 0 "PASSed if we get this far"
}

################################################################################
# main
################################################################################

TST_TOTAL=3

# standard setup
tc_setup

test01 &&
test02 &&
test03
