#!/bin/bash
################################################################################
#                                                                              #
#  (C) Copyright IBM Corp. 2003						       #
#                                                                              #
#  This program is free software;  you can redistribute it and#or modify       #
#  it under the terms of the GNU General Public License as published by        #
#  the Free Software Foundation; either version 2 of the License, or           #
#  (at your option) any later version.                                         #
#                                                                              #
#  This program is distributed in the hope that it will be useful, but         #
#  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY  #
#  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    #
#  for more details.                                                           #
#                                                                              #
#  You should have received a copy of the GNU General Public License           #
#  along with this program;  if not, write to the Free Software                #
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA     #
#                                                                              #
################################################################################
#
# File :       ln_tests.sh
#
# Description: This program tests basic functionality of ln program
#
# Author:      Manoj Iyer  manjo@mail.utexas.edu
#
# History:     Sept 26 2003 - Re-created - Manoj Iyer
#              The original testcase was overkill so I replaced it with 
#              a more simpler one.
#		08 Dec 2003 (rcp) Rewrote to avoid need for "file" command.
#				Added hard linktest to go with soft link test.
#		08 Dec 2003 (rcp) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

#
# test01	use ln to make soft link
#
function test01()
{
	tc_register	"use ln to make soft link"
	tc_exec_or_break ls || return

	echo "original file" > $TCTMP/orig_file
	ln -s $TCTMP/orig_file $TCTMP/soft_link >$stdout 2>$stderr
	tc_fail_if_bad $? "bad response from ln command" || return

	# should show orig_file inode
	local aaa=`ls -liL $TCTMP/soft_link`
	set $aaa
	local inodeaaa=$1

	# should ALSO show orig_file inode
	local bbb=`ls -li $TCTMP/orig_file`
	set $bbb
	local inodebbb=$1

	# ensure inodes match when using -L optoin of ls
	[ $inodeaaa -eq $inodebbb ]
	tc_fail_if_bad $? "soft link did not point to proper inode " \
		"expected $inodeaaa to equal $inodebbb" || return

	# should NOT show orig_file inode
	local ccc=`ls -li $TCTMP/soft_link`
	set $ccc
	local inodeccc=$1

	# ensure inodes do NOT match without -L option of ls
	[ $inodeaaa -ne $inodeccc ]
	tc_pass_or_fail $? "soft link had same inode as original file!" \
			"perhaps a hard link was created"
}

#
# test02	use ln to make hard link
#
function test02()
{
	tc_register	"use ln to make hard link"
	tc_exec_or_break ls || return

	echo "original file" > $TCTMP/orig_file
	ln $TCTMP/orig_file $TCTMP/hard_link >$stdout 2>$stderr
	tc_fail_if_bad $? "bad response from ln command" || return

	# should show orig_file inode
	local aaa=`ls -li $TCTMP/hard_link`
	set $aaa
	local inodeaaa=$1

	# should ALSO show orig_file inode
	local bbb=`ls -li $TCTMP/orig_file`
	set $bbb
	local inodebbb=$1

	# ensure orig and link have same inode
	[ $inodeaaa -eq $inodebbb ]
	tc_pass_or_fail $? "isoft link did not point to proper inode " \
		"expected $inodeaaa to equal $inodebbb"
}

# 
# Function: main
# 

TST_TOTAL=2

tc_setup
test01
test02
