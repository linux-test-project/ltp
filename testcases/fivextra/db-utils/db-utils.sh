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
# File :	db-utils.sh
#
# Description:	test the Berkeley Database utilities
#		Curently this tets db_dump and db_load. The complete list is:
#			berkeley_db_svc		RPC server utility
#			db_archive		Archival utility
#			db_checkpoint		Transaction checkpoint utility
#			db_deadlock		Deadlock detection utility
#			db_dump	Database 	dump utility
#			db_load	Database 	load utility
#			db_printlog		Transaction log display utility
#			db_recover		Recovery utility
#			db_stat	Statistics 	utility
#			db_upgrade		Database upgrade utility
#			db_verify		0Verification utility
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Mar 18 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		15 Dec 2003 (robert) updated to tc_utils.source

################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# any utility functions specific to this file can go here
################################################################################

################################################################################
# the testcase functions
################################################################################

#
# test01	tests db_load
#
function test01()
{
	tc_register	"db_load"
	tc_exec_or_break cat rm || return
#
	# raw data for input to database
	cat > $TCTMP/animals.txt <<-EOF
		key1
		cat
		key2
		dog
		key3
		pig
		key4
		horse
	EOF
#
	# load data into database animals.db
	rm $TCTMP/animals.db &>/dev/null
	cat $TCTMP/animals.txt | \
		db_load -T -t hash $TCTMP/animals.db 2>$stderr
	tc_pass_or_fail $? "bad return from db_load" "rc=$?"
}

#
# test02	tests db_dump
#
function test02()
{
	tc_register	"db_dump"
	tc_exec_or_break || return
	db_dump -p $TCTMP/animals.db > $TCTMP/animals.out 2>$stderr
	tc_fail_if_bad $? "bad return from db_dump of animals.db" "rc=$?" \
		|| return
#
	cat $TCTMP/animals.out | db_load $TCTMP/animals2.db 2>$stderr
	tc_fail_if_bad $? "bad return from db_load into animals2.db" "rc=$?" \
		|| return
#
	db_dump -p $TCTMP/animals2.db > $TCTMP/animals2.out 2>$stderr
	tc_fail_if_bad $? "bad return from db_dump of animals2.db" "rc=$?" \
		|| return
#
	diff $TCTMP/animals.out $TCTMP/animals2.out 2>$stderr
	tc_pass_or_fail $? "bad compare of animals.txt and animals2.txt" \
			"expected"$'\n'"`cat $TCTMP/animals.out`" \
			"got:"$'\n'"`cat $TCTMP/animals2.out`"

}
################################################################################
# main
################################################################################

TST_TOTAL=2

tc_setup

test01 && \
test02
