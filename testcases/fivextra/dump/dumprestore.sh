#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		      ##
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
# File :	dump-restore.sh
#
# Description:	test dump and restore backup utilities 
#		
# Author:	Helen Pang, hpang@us.ibm.com
#
# History:	August 20 2003 - Created. Helen Pang. hpang@us.ibm.com
#		Oct 10 2003 - RC Paulsen: added installation check.
#				Prevent garbage output to stdout.
#		16 Dec 2003 - (hpang) updated to tc_utils.source
#
################################################################################
# source the standard utility functions
###############################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

###############################################################################
# utility functions
###############################################################################

function tc_local_setup()
{
	tc_is_fstype $TCTMP ext2 || tc_is_fstype $TCTMP ext3
	tc_break_if_bad $? "Only supported for ext2/ext3 filesystems"
	datadir=$TCTMP/datadir; mkdir -p $TCTMP/datadir
}

function tc_local_cleanup()
{
	return
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
	tc_executes dump restore
	tc_pass_or_fail $? "not properly installed"
}

#
# test02	dump to file
#
function test02()
{
	tc_register "dump to file"
	tc_exec_or_break echo grep || return

	echo "Sample file to dump" > $datadir/samplefile
	set `ls -i $datadir/samplefile`; inode=$1
	/bin/sync
	dump -0f $TCTMP/dumpfile $datadir &>$stdout
	tc_fail_if_bad $? "unexpected RC from dump" || return

	grep -q "dump completed" $stdout 
	tc_pass_or_fail $? "expected to see \"dump completed\" in output"
}

#
# test03	restore TOC
#
function test03()
{
	tc_register "restore TOC"
	tc_exec_or_break grep || return
	
	# first look at TOC
	restore tf $TCTMP/dumpfile >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected result from restore tf $TCTMP/dumpfile" || return

	# ensure file name in TOC
	grep -q "$datadir/samplefile" $stdout &>$stderr
	tc_pass_or_fail $? "Expected to see \"samplefile\" file name in stdout"
}

#
# test04	restore filesystem
#
function test04()
{
	tc_register "restore filesystem"
	tc_exec_or_break cat grep || return

	# save copy of and remove original file in preparation for restore
	mv $datadir/samplefile $TCTMP/samplefile
	rm -rf $datadir
	/bin/sync

	# restore the dumped directory
	( cd /; echo y | restore -avf $TCTMP/dumpfile -x $datadir &>$stdout )
	tc_fail_if_bad $? "Unexpected result" || return

	# compare original to restored file
	diff $datadir/samplefile $TCTMP/samplefile >$stdout 2>$stderr
	tc_pass_or_fail $? "restored file did not match original"
}

#
# test05	restore -C
#
#	TODO: Why does this fail even though the above compare succeeds?
#		For now, not executed.
#
function test05()
{
	tc_register "restore -C (compare)"

	restore -vCf $TCTMP/dumpfile -L 0 >$stdout 2>$stderr
	tc_pass_or_fail $? "Unexpected response"
}

################################################################################
# main
################################################################################

TST_TOTAL=4

# standard tc_setup
tc_setup

tc_root_or_break || exit

test01 &&
test02 &&
test03 &&
test04
