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
# File :	canna.sh
#
# Description:	This script invokes all the canna tests scripts
#
# Author:	Shoji Sugiyama (shoji@jp.ibm.com)
#
# History:	Jun 11 2003 - Initial version created. 
#		05 Jan 2004 - (rcp) updated to tc_utils.source
#		20 Jul 2004 - (James) change eval $file to eval ./$file
# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source
tc_setup
tc_register	"canna"

#
# Run test
#
function runtest()
{
	#
	# Check if target package has been installed.
	#
	tc_exec_or_break cannastat mkdic lsdic rmdic || return

	#
	# Execute a series of test cases
	#
	rc=0;
	pwd=`pwd`
	cd $LTPBIN/canna_dir
	for file in `ls *.sh | grep -v canna.sh`
	do
	#	echo $file
		eval ./$file || rc=$?
	done
	cd $pwd

	tc_pass_or_fail $rc "one of the above FAILed or BROK"
}

#
# Test main.
#
runtest
