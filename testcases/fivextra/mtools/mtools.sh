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
# File :	mtools.sh
#
# Description:	test mtools support.
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Sep 16 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		16 Dec 2003 - (rcp) updated to tc_utils.source

################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

do_umount="no"	# set to yes after successful mount. checked in tc_local_cleanup
mtools_commands="mattrib mbadblocks mcat mcd mcopy mdel mdeltree mdir
		mdu mformat minfo mlabel mmd mmount mmove mpartition
		mrd mren mshowfat mtoolstest mtype mzip"

################################################################################
# utility functions
################################################################################

#
# tc_local_setup	setup for this testcase
#			Be sure to run test01 first.
#
function tc_local_setup()
{
	image_file=$TCTMP/mtools.img
	mnt=$TCTMP/mnt
	mkdir $mnt
	export MTOOLSRC=$TCTMP/mtools.conf
}

#
# tc_local_cleanup	cleanup specific to this testcase
#
function tc_local_cleanup()
{
	[ "$do_umount" = "yes" ] && umount $mnt
}

################################################################################
# the testcase functions
################################################################################

#
# test01	install check
#
function test01()
{
	tc_register	"install check"
	tc_executes $mtools_commands
	tc_pass_or_fail $? "mtools support not installed properly"
}

#
# test02	mtoolstest
#
function test02()
{
	tc_register	"mtoolstest"
	mtoolstest > $stdout 2>$stderr
	tc_pass_or_fail $? "mtoolstest failed" || return
	cp $stdout $MTOOLSRC
	cat >> $MTOOLSRC <<-EOF
	drive T:
		file="$image_file" fat_bits=16
		tracks=0 heads=0 sectors=0 hidden=0
		offset=0x0
		partition=0
		mformat_only 
	EOF
}

#
# terst03	mformat
#
function test03()
{
	tc_register	"mformat"
	dd if=/dev/zero of=$image_file count=4096 &>/dev/null
	mformat -f 2880 t: >$stdout 2>$stderr
	tc_pass_or_fail $? "mformat failed"
}

#
# terst04	mcopy
#
function test04()
{
	tc_register	"mcopy"
	mcopy $0 t: >$stderr 2>$stdout
	tc_fail_if_bad $? "mcopy failed" || return
	#
	do_umount="yes"
	mount -t msdos $image_file $mnt -o loop >$stdout 2>$stderr
	tc_fail_if_bad $? "msdos filesystem created by mformat failed to mount"
	#
	diff $0 $mnt/`basename $0` >$stdout 2>$stderr
	tc_pass_or_fail $? "diff of copied file failed"
}

################################################################################
# main
################################################################################

TST_TOTAL=4

tc_setup

tc_root_or_break || exit

test01 &&
test02 &&
test03 &&
test04
