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
# File :	   mkudffs_tests.sh
#
# Description: This program tests basic functionality of mkudffs command
#
# Author:	   Manoj Iyer  manjo@mail.utexas.edu
#
# History:	   July 28 2003 - created - Manoj Iyer
#			   Oct	14 2003 - Modified - Manoj Iyer
#				- changed from block device major #3 to major #1
#		Oct 28 2003 (RC Paulsen) fixed BUG 4802 per Ryan Harper's patch
#				(use loopback device instead of mknod)
#				also general cleanup
#		08 Jan 2004 - (RR) updated to tc_utils.source
#		03 May 2004 0 (rcp) add chkudf test.

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# utility function
################################################################################

#
# tc_local_setup
#
function tc_local_setup()
{
	tc_root_or_break || return
	tc_exec_or_break diff dd mount mkdir grep || return
}

#
# tc_local_cleanup
# 
tc_local_cleanup()
{
	umount $TCTMP/tst_udf_mnt &>/dev/null
}

################################################################################
# testcase functions
################################################################################

#
# test01	installation check
#
function test01()
{
	tc_register	"installation check"
	tc_executes mkudffs
	tc_pass_or_fail $? "not properly installed"
}

#
# test02	make a udf filesystem
#
function test02()
{
	tc_register    "mkudffs command"

	# create udf file system on loopback device
	dd if=/dev/zero of=$TCTMP/tst_udf bs=1024k count=8 &>$stdout
	local command="mkudffs --media-type=dvd $TCTMP/tst_udf"
	$command >$TCTMP/tst_mkudffs.out 2>$stderr
	tc_fail_if_bad $? "unexpected response from command" "$command" || return
	
	# create expected output.
	cat <<-EOF > $TCTMP/tst_mkudffs.exp
		start=0, blocks=16, type=RESERVED
		start=16, blocks=3, type=VRS
		start=19, blocks=237, type=USPACE
		start=256, blocks=1, type=ANCHOR
		start=257, blocks=16, type=PVDS
		start=273, blocks=1, type=LVID
		start=274, blocks=3565, type=PSPACE
		start=3839, blocks=1, type=ANCHOR
		start=3840, blocks=239, type=USPACE
		start=4079, blocks=16, type=RVDS
		start=4095, blocks=1, type=ANCHOR
	EOF

	# if they are different declare fail, else declare pass.
	diff -wB $TCTMP/tst_mkudffs.out $TCTMP/tst_mkudffs.exp >$stdout 2>$stderr
	tc_pass_or_fail $? "miscompare on output from command" "$command"
}

#
# test03	mount the udf filesystem created above
#
function test03()
{
	tc_register	"mount the udf filesystem"

	# create a temporary mount point.
	mkdir -p $TCTMP/tst_udf_mnt 2>$stderr >$stdout
	tc_fail_if_bad $? "failed creating temporary mount point" || return

	# mount the new device as a udf device, this is the ultimate test
	mount -t udf -oloop $TCTMP/tst_udf $TCTMP/tst_udf_mnt >$stdout 2>$stderr
	tc_fail_if_bad $? "mount command failed" || return

	mount >$stdout 2>$stderr
	grep -q "$TCTMP/tst_udf_mnt" $stdout 2>>$stderr
	tc_pass_or_fail $? "expected to see" \
		"\"$TCTMP/tst_udf_mnt\" in stdout"
}

#
# test04	read/write mounted filesystem
#
function test04()
{
	tc_register	"read/write mounted filesystem"

	echo "hello sailor" > $TCTMP/tst_udf_mnt/hello.txt
	tc_fail_if_bad $? "couldn't write to udf filesystem" || return

	ls $TCTMP/tst_udf_mnt/ >$stdout 2>$stderr &&
	grep -q "hello.txt" $stdout 2>>$stderr
	tc_fail_if_bad $? "file not created on udf filesystem" || return

	grep -q "hello sailor" $TCTMP/tst_udf_mnt/hello.txt 2>$stderr
	tc_pass_or_fail $? "data mis-compare on udf filesystem"
}

#
# test05	chkudf command
#
function test05()
{
	tc_register	"chkudf"

	tc_executes chkudf
	tc_fail_if_bad	$? "chkudf not installed" || return

	chkudf $TCTMP/tst_udf >$stdout 2>$stderr
	tc_pass_or_fail $? "unexpected response"
}

# 
# main
#

TST_TOTAL=5
tc_setup

test01 &&
test02 &&
[ "$1" = "chkudf" ] && test05 &&
test03 &&
test04 
