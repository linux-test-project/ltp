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
# File :	scp.sh
#
# Description:	Test basic functionality of scp - secure copy
#		(remote file copy program)
#
# Author:	Yu-Pao Lee, yplee@us.ibm.com
#
# History:	Mar 06 2003 - Created. Yu-Pao Lee, yplee @us.ibm.com
#		Sep 30 2003 - RC Paulsen: force command line prompt to 
#				known value. Put output in $stdout.
#		Nov 25 2003 - Andrew Pham: increase timeout in expect
#			      script to 50 in testcase # 2
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
	tc_exec_or_break grep expect hostname cat ls su chmod diff || return

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
	tc_executes scp
	tc_pass_or_fail $? "scp not installed"
}

#
# test02	scp functionality"
#
function test02()	# scp - secure copy a file
{
	tc_register "scp - secure copy a file"
	
	# create expect file to issue scp command
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
			"$prompt" { send "mkdir tmp_dir1\r" }
		}
		expect {
			timeout abort
			"$prompt" { send "touch aaa\r" }
		}
		expect {
			timeout abort
			"$prompt" { send "scp aaa $id@$host:./tmp_dir1\r"}
		}
		expect {
			timeout abort
			"(yes/no)?" { send "yes\r" }
		}
		expect {
			timeout abort
			"password:" { send "password\r" }
		}
		expect {
			timeout abort
			"$prompt" { send "ls ./tmp_dir1/ > $TCTMP/file1\r" }
		}
		expect {
			timeout abort
			"$prompt" { send "exit\r" }
		}
		expect eof
		send_user "all done\n"
	EOF
	chmod +x $TCTMP/expcmd
	
	$TCTMP/expcmd >$stdout 2>$stderr
	tc_fail_if_bad $? "expect file failed." || return
	
	# create the expected output
	cat > $TCTMP/file2 <<-EOF
		aaa
	EOF
	
	diff -w -B $TCTMP/file1 $TCTMP/file2 >$stdout 2>$stderr
	tc_pass_or_fail $? "scp failed." "expected to see aaa"
}


function test03()	# scp - to secure copy a file using keyfiles
{
	tc_register "scp using keyfiles"

	tc_exec_or_break ssh-keygen || return

	# create expect file to issue ssh-keygen and rename public
	# keyfile, then issue scp. It should require no password for scp
	local expcmd=`which expect`
	local host=`hostname -s`	# ensure short form of hostname
	local prompt='xxx'
	echo "PS1=$prompt" >> /home/$temp_user/.bashrc
	cat > $TCTMP/expcmd <<-EOF
		#!$expcmd -f
		set timeout 50
		proc abort {} { exit 1 }
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
			"$prompt" { send "touch bbb\r" }
		}
		expect {
			timeout abort
			"$prompt" { send "mkdir tmp_dir2\r" }
		}
		expect {
			timeout abort
			"$prompt" { send "cd .ssh\r" }
		}
		expect {
			timeout abort
			"$prompt" { send "mv id_dsa.pub authorized_keys2\r"}
		}
		expect {
			timeout abort
			"$prompt" {
				send "scp ~/bbb $temp_user@$host:tmp_dir2/\r"
			}	
		}
		expect {
			timeout	abort
			"$prompt" { send "ls ~/tmp_dir2/ > $TCTMP/fileA\r"}
			"password:" abort
		}
		expect {
			timeout abort
			"$prompt" { send "exit\r" }
		}
		expect eof
		send_user "all done\n"
	EOF
	chmod +x $TCTMP/expcmd
	
	$TCTMP/expcmd >$stdout 2>$stderr
	tc_fail_if_bad $? "expect file failed." || return

	# create the expected output
	cat > $TCTMP/fileB <<-EOF
	bbb
	EOF
	
	diff -w -B $TCTMP/fileA $TCTMP/fileB >$stdout 2>$stderr
	tc_pass_or_fail $? "scp failed." "expected to see bbb"
}

################################################################################
# main
################################################################################

TST_TOTAL=3

tc_setup			# standard setup

chown -R $temp_user $TCTMP

test01 &&
test02 &&
test03
