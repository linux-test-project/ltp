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
# File :	template_sh
#
# Description:	This is a template that can be used to develop shell script
#		testcases using the the LTP-harness command APIs.
#		It tests the su command as an example.
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Oct 22 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#			TODO: Get nhfsstone working (test09)
#		16 Dec 2003 - (rcp) updated to tc_utils.source
#		24 Feb 2004 (rcp) remove nhfsstone (BUG 5947)

################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

# NOTE: PICK ONE!
nfsserverinit="/etc/init.d/nfs"		# use for RedHat
nfsserverinit="/etc/init.d/nfsserver"	# use for MCP/SuSE/UL

export_me=""				# directory path set by tc_local_setup
mount_me=""				# directory path set by tc_local_setup
local_setup_ok="no"			# set to yes by tc_local_setup
restart_nfsserver="no"			# set to yes by tc_local_setup and test02
must_unexport="no"			# set to yes by test03
must_unmount="no"			# set to yes by test04

################################################################################
# utility functions specific to this script
################################################################################

#
# tc_local_setup
#
function tc_local_setup()
{
	tc_root_or_break || return	# this tc must be run as root
	tc_exec_or_break mount umount mkdir grep diff || return
	[ -e $nfsserverinit ] &&
		$nfsserverinit status &>/dev/null &&
		restart_nfsserver="yes"

	# create exportable directory, file and mount point
	export_me=$TCTMP/export_me; mkdir $export_me
	mount_me=$TCTMP/mount_me; mkdir $mount_me
	echo "This is test data" > $export_me/test_file
	local_setup_ok="yes"
}

#
# tc_local_cleanup
#
function tc_local_cleanup()
{
	[ $local_setup_ok = "yes" ] || return
	[ $must_unmount = "yes" ] &&
		umount $mount_me &>/dev/null
	[ $must_unexport = "yes" ] &&
		exportfs -u localhost:$export_me &>/dev/null
	[ -f $TCTMP/exports ] &&
		cp -a $TCTMP/exports /etc
	[ -e $nfsserverinit ] && {
		$nfsserverinit stop &>/dev/null
		[ $restart_nfsserver = "yes" ] &&
			$nfsserverinit start &>/dev/null
	}
}

################################################################################
# the testcase functions
################################################################################

#
# test01	check that portmap is installed
#
function test01()
{
	tc_register	"installation check"
	tc_executes $nfsserverinit exportfs nfsstat showmount
	tc_pass_or_fail $? "nfs-utils not installed properly"
}

#
# test02	(re)start the nfs server
#
function test02()
{
	tc_register	"(re)start nfs server"

	# if not already running, remember to stop it when done
	$nfsserverinit status >$stdout 2>$stderr && restart_nfsserver="yes"

	$nfsserverinit restart >$stdout 2>$stderr
	tc_pass_or_fail $? "could not start nfs server"
}

#
# test03	export a directory
#
function test03()
{
	tc_register	"export a directory"

	sleep 2

	# save original exports file and set up our export
	[ -f /etc/exports ] && mv /etc/exports $TCTMP/
	echo "$export_me localhost(rw,no_root_squash,sync)" > /etc/exports

	# export
	must_unexport="yes"	# remember to unexport it later
	exportfs -a >$stdout 2>$stderr
	tc_pass_or_fail $? "unexpected response to exportfs -a"
	sync; sync; sync;
}

#
# test04	read from nfs-mounted filesystem
#
function test04()
{
	tc_register	"read from nfs-mounted filesystem"

	must_unmount="yes"

	local command="mount -t nfs localhost:$export_me $mount_me"
	$command >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response to $command" || return

	diff $export_me/test_file $mount_me/test_file >$stdout 2>$stderr
	tc_pass_or_fail $? "mismatch between exported and mounted file"
}

#
# test05	write to nfs-mounted filesystem
#
function test05()
{
	tc_register	"write to nfs-mounted filesystem"

	echo "more test data" > $mount_me/more_test_data
	tc_fail_if_bad $? "could not write to nfs-mounted filesystem" || return

	diff $mount_me/more_test_data $export_me/more_test_data >$stdout 2>$stderr
	tc_pass_or_fail $? "mismatch between exported and mounted file"
}

#
# test06	showmount
#
function test06()
{
	tc_register	"showmount"

	showmount >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from showmount" || return

	local expected="localhost"
	grep -q $expected $stdout 2>$stderr
	tc_pass_or_fail $? "expected to see $expected in stdout"
}

#
# test07	nfsstat
#
function test07()
{
	tc_register	"nfsstat"

	nfsstat >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from nfsstat" || return

	grep -qi "client nfs" $stdout 2>$stderr
	tc_fail_if_bad $? "expected to see client nfs in stdout" || return

	grep -qi "server nfs" $stdout 2>$stderr
	tc_fail_if_bad $? "expected to see client nfs in stdout" || return

	grep -qi "client rpc" $stdout 2>$stderr
	tc_fail_if_bad $? "expected to see client nfs in stdout" || return

	grep -qi "server rpc" $stdout 2>$stderr
	tc_pass_or_fail $? "expected to see client nfs in stdout"
}

#
# test08	minor stress test
#
function test08()
{
	tc_register	"minor stress test"
	
	declare -i local i=0
	declare -i local MAX=500
	declare -i local TOTAL
	let TOTAL=2*$MAX
	tc_info "$TCNAME: NFS client and server each create $MAX files"
	while [ $i -lt $MAX ] ; do

		# client create file
		cp $0 $mount_me/client_file_$i 2>$stderr
		echo "client file nbr $i" >> $mount_me/client_file_$i 2>$stderr
		tc_fail_if_bad $? "couldn't create client_file_$i" || return

		# server create file
		cp $0 $export_me/server_file_$i 2>$stderr
		echo "server file nbr $i" >> $export_me/server_file_$i 2>$stderr
		tc_fail_if_bad $? "couldn't create server_file_$i" || return
		let ++i
	done

	# an attempt to defeat and caching by client or server
	mkdir $TCTMP/server_files; cp $export_me/* $TCTMP/server_files/
	mkdir $TCTMP/client_files; cp $mount_me/* $TCTMP/client_files/

	# now compare the files
	tc_info "$TCNAME: Compare all $TOTAL files"
	diff $TCTMP/server_files $TCTMP/client_files >$stdout 2>$stderr
	tc_pass_or_fail $? "mismatch between exported and mounted file"
}

################################################################################
# main
################################################################################

TST_TOTAL=8

tc_setup

test01 &&
test02 &&
test03 &&
test04 &&
test05 &&
test06 &&
test07 &&
test08
