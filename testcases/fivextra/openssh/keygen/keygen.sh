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
# File :	keygen.sh
#
# Description:	Test ssh-keygen - authentication key generation
#
# Author:	Yu-Pao Lee, yplee@us.ibm.com
#
# History:	Mar 03 2003 - Created. Yu-Pao Lee, yplee@us.ibm.com
#		Oct 01 2003 - RC Paulsen. force known prompt; cleanup.
#		17 Dec 2003 - (rcp) updated to tc_utils.source
#		23 Jan 2004 (rcp) increases timeout in expect script in an
#				atempt to avoid intermittent failures.
#		12 May 2004 (rcp) Use ssh-keygen insgtead of keygen. (How did
#				this EVER work?)

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# global variables
started_sshd="no"

################################################################################
# utility functions for this testcase
################################################################################

function tc_local_cleanup()
{
	if [ "$started_sshd" = "yes" ] ; then
		/etc/init.d/sshd stop
		tc_info "Stopped sshd."
	fi
}

function tc_local_setup()
{
	tc_exec_or_break grep diff hostname expect cat ls su ssh ssh-keygen chmod || return

	tc_root_or_break || return 

	tc_add_user_or_break || return

	# check if sshd is up
	if ! /etc/init.d/sshd status | grep -q running ; then
		/etc/init.d/sshd start | grep -q done 2>$stderr
		tc_fail_if_bad $? "sshd did not start." || return
		tc_info "Started sshd for this testcase"
		started_sshd="yes"
	fi	
	tc_info "sshd is up and running."
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
	tc_executes ssh-keygen
	tc_pass_or_fail $? "ssh-keygen not installed"
}

#
# test02	ssh-keygen functionality
#
function test02()	# ssh-keygen to generate keys
{
	tc_register "ssh-keygen -t dsa to generate keys used with DSA authentication"

	# create expect file to issue ssh-keygen to generate dsa keys 
	local expcmd=`which expect`
	local prompt=':::'
	echo "PS1=$prompt" >> /home/$temp_user/.bashrc
	cat > $TCTMP/expcmd <<-EOF
		#!$expcmd -f
		proc abort {} { exit 1 }
		set timeout 30
		spawn su - $temp_user
		expect {
			timeout abort  
			"$prompt" { send "ssh-keygen -t dsa\r" }
		}
		expect {
			timeout abort
			"Enter file" { send "\r" }
		}
		expect {
			timeout abort
			"passphrase" { send "\r" }
		}
		expect {
			timeout abort
			"same passphrase" { send "\r" }
		}
		expect {
			timeout abort
			"$prompt" { send 'exit\r" }
		}
		expect eof
		send_user "all done\n"
	EOF
	chmod +x $TCTMP/expcmd

	tc_info "running \"ssh-keygen -t dsa\". This takes a few seconds ..."
	
	$TCTMP/expcmd >$stdout 2>$stderr
	tc_fail_if_bad $? "expect file failed." || return
	
	ls /home/$temp_user/.ssh/ > $TCTMP/keysfile1

	# create the expected output
	cat > $TCTMP/keysfile2 <<-EOF
		id_dsa
		id_dsa.pub
	EOF
	
	diff -w -B $TCTMP/keysfile1 $TCTMP/keysfile2 >$stdout 2>$stderr
	tc_pass_or_fail $? "ssh-keygen failed." "expected to see id_dsa and id_dsa.pub"
}

################################################################################
# main
################################################################################

TST_TOTAL=2

tc_setup			# standard setup

test01 &&
test02

