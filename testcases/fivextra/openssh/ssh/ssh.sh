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
# File :	ssh.sh
#
# Description:	Test OpenSSH SSH client (remote login program)
#
# Author:	Yu-Pao Lee, yplee@us.ibm.com
#
# History:	Mar 03 2003 - Created. Yu-Pao Lee. yplee@us.ibm.com
#		Sep 30 2003 - RC Paulsen: force command line prompt to 
#				known value. Put output in $stdout.
#		17 Dec 2003 - (rcp) updated to tc_utils.source

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
		tc_info "Stopped sshd"
	fi
}

function tc_local_setup()
{
	tc_exec_or_break grep hostname expect cat ls su ssh ssh-keygen chmod || return

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
	tc_executes ssh
	tc_pass_or_fail $? "scp not installed"
}

#
# test02	ssh functionality
#
function test02()	# ssh to login to a machine
{
	tc_register "ssh to login to a machine"

	# create expect file to create a login session to a machine
	local expcmd=`which expect`
	local host=`hostname -s`	# ensure short form of hostname
	local prompt=':::'
	echo "PS1=$prompt" >> /home/$temp_user/.bashrc
	cat > $TCTMP/expcmd <<-EOF
		#!$expcmd -f
		proc abort {} { exit 1 }
		spawn su - $temp_user
		expect {
			timeout abort
			"$prompt" { send "ssh $temp_user@$host\r" }
		}
		expect {
			"(yes/no)?" { send "yes\r" }
		}
		expect {
			timeout abort
			"password:" { send "password\r" }
		}
		set timeout 60
		expect {
			timeout abort
			"$prompt" { send "exit\r" }
		}
		set timeout 5
		expect {
			timeout abort
			"$prompt" { send "exit\r" }
		}
		send_user "all done\n"
	EOF
	chmod +x $TCTMP/expcmd
	
	$TCTMP/expcmd >$stdout 2>$stderr
	tc_pass_or_fail $? "ssh failed." 
}

################################################################################
# main
################################################################################

TST_TOTAL=2

tc_setup			# standard setup

test01 &&
test02
