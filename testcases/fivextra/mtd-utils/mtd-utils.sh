#!/bin/sh
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2004                                               ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :   mtd-utils.sh
#
# Description: This program tests basic functionality of mtd-utils command.
#
# Author:   Gong Jie <gongjie@cn.ibm.com>
#
# History:      Jun 29 2004 - created - Gong Jie

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

#
# local setup
#
function tc_local_setup()
{
	tc_root_or_break || return
	tc_exec_or_break rm grep mknod modprobe rmmod || return
	rm -f /dev/mtd0 && mknod -m 644 /dev/mtd0 c 90 0
	rm -f /dev/mtdblock0 && mknod -m 644 /dev/mtdblock0 b 31 0
	modprobe mtdcore
	# 32meg flash Ipaqs
	modprobe mtdram total_size=32768 erase_size=256
	modprobe mtdblock jffs2
}

#
# local cleanup
#
function tc_local_cleanup()
{
	rmmod jffs2 mtdblock mtdram mtdcore >$stdout 2>$stderr
}

#
# test1:  installcheck  Installation check
#
function installcheck()
{
	tc_register "mtd-utils installation check"
	tc_executes doc_loadbios einfo erase eraseall fcp ftl_check \
		ftl_format jffs2dump lock mkfs.jffs mkfs.jffs2 \
		mtd_debug nanddump nandwrite unlock
	tc_pass_or_fail $? "mtd-utils not properly installed"
}

#
# test2: doc_loadbios test
#
function doc_loadbios_test()
{
	tc_register "doc_loadbios test"
	tc_exist_or_break mtd_files/foo.img
	doc_loadbios /dev/mtd0 mtd_files/foo.img >$stdout 2>$stderr
	tc_pass_or_fail $? "doc_loadbios failed"
}

#
# test3: einfo test
#
function einfo_test()
{
	tc_register "einfo test"
	einfo /dev/mtd0 >$stdout 2>$stderr
	tc_pass_or_fail $? "einfo failed"
}

#
# test4: erase test
#
function erase_test()
{
	tc_register "erase test"
	erase /dev/mtd0 >$stdout 2>$stderr
	tc_pass_or_fail $? "erase failed"
}

#
# test5: eraseall test
#
function eraseall_test()
{
	tc_register "eraseall test"
	eraseall -j /dev/mtd0 >$stdout 2>$stderr
	tc_pass_or_fail $? "eraseall failed"
}

#
# test6: fcp test
#
function fcp_test()
{
	tc_register "fcp test"
	tc_exist_or_break mtd_files/foo.img
	fcp -v mtd_files/foo.img /dev/mtd0 >$stdout 2>$stderr
	tc_pass_or_fail $? "fcp failed"
}

#
# test7: ftl_format test
#
function ftl_format_test()
{
	tc_register "ftl_format test"
	ftl_format -s 1 -r 5 -b 0 /dev/mtd0 >$stdout 2>$stderr
	tc_pass_or_fail $? "ftl_format failed"
}

#
# test8: ftl_check test
#
function ftl_check_test()
{
	tc_register "ftl_check test"
	ftl_check -v /dev/mtd0 >$stdout 2>$stderr
	tc_pass_or_fail $? "ftl_check failed"
}

#
# test9: mkfs.jffs test
#
function mkfs_jffs_test()
{
	tc_register "mkfs.jffs test"
	mkfs.jffs -d . -o /dev/mtd0 -v9 >$stdout 2>&1
	tc_pass_or_fail $? "mkfs.jffs failed"
}

#
# test10: mkfs.jffs2 test
#
function mkfs_jffs2_test()
{
	tc_register "mkfs.jffs2 test"
	mkfs.jffs2 -d . -o /dev/mtd0 -v >$stdout 2>$stderr
	tc_pass_or_fail $? "mkfs.jffs2 failed"
}

#
# test11: jffs2dump test
#
function jffs2dump_test()
{
	tc_register "jffs2dump test"
	jffs2dump -cv /dev/mtd0 >$stdout 2>$stderr
	tc_pass_or_fail $? "jffs2dump failed"
}

#
# test12: lock test
#
function lock_test()
{
	tc_register "lock test"
	lock /dev/mtd0 0 -1 >$stdout 2>$stderr
	tc_pass_or_fail $? "lock failed"
}

#
# test13: ulock test
#
function unlock_test()
{
	tc_register "unlock test"
	unlock /dev/mtd0 >$stdout 2>$stderr
	tc_pass_or_fail $? "unlock failed"
}

#
# test14: mtd_debug test
#
function mtd_debug_test()
{
	tc_register "mtd_debug test"
	mtd_debug info  /dev/mtd0 >$stdout 2>$stderr
	tc_fail_if_bad $? "mtd_debug info failed"
	mtd_debug read  /dev/mtd0 0 33554432 $TCTMP/mtd_debug.out \
		>$stdout 2>$stderr
	tc_fail_if_bad $? "mtd_debug read failed"
	mtd_debug write /dev/mtd0 0 33554432 $TCTMP/mtd_debug.out \
		>$stdout 2>$stderr
	tc_fail_if_bad $? "mtd_debug write failed"
	mtd_debug erase /dev/mtd0 0 33554432 >$stdout 2>&1
	tc_pass_or_fail $? "mtd_debug erase failed"
}

#
# test15: nanddump test
#
function nanddump_test()
{
	tc_register "nanddump test"
	nanddump /dev/mtd0 $TCTMP/nanddump.out 0 33554432 >$stdout 2>$stderr
	tc_pass_or_fail $? "nanddump failed"
}

#
# test16: nandwrite test
#
function nandwrite_test()
{
	tc_register "nandwrite test"
	nanddump /dev/mtd0 $TCTMP/nanddump.out >$stdout 2>$stderr
	tc_pass_or_fail $? "nandwrite failed"
}
			
#
# main
#
TST_TOTAL=12
tc_setup

installcheck || exit
doc_loadbios_test
einfo_test
erase_test
eraseall_test
fcp_test
ftl_format_test
ftl_check_test
mkfs_jffs_test
mkfs_jffs2_test
jffs2dump_test
#lock_test
#unlock_test
mtd_debug_test
#nanddump_test
#nandwrite_test
