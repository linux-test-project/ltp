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
# File :	expect.sh
#
# Description:	Test basic functionality/commands of Expect program
#
# commands tested: expect puts send send_user send_error set unset spawn lindex	
#		   proc catch error "info exists" " file exists"	
#
# Author:	Yu-Pao Lee, yplee@us.ibm.com
#
# History:	Mar 11 2003 - Created. Yu-Pao Lee, yplee @us.ibm.com
#		Oct 29 2003 - RC Paulsen: minor cleanup.
#		17 Nov 2003 - (RCP) add "sleep 2" to avoid timing problem with
#				expect and passwd. BUG 5203.
#		17 Nov 2003 - (rcp) updated to tc_utils.source
#               02 Dec 2003 - (rcp) BUG 5433

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# the testcase functions
################################################################################

function test01()	# installation check
{
	tc_register	"installation check"
	tc_executes expect
	tc_pass_or_fail $? "not properly instaled"
}

function test02()	# test expect, spawn, send, send_user, and lindex
{
	tc_register	"test expect/spawn/send/send_user/lindex"
	local password="nobody1"
	
	tc_exec_or_break scp cat chmod || return

	# create expect file to test Expect commands
	export HOST=`hostname -s`
	local expcmd=`which expect`
	cat > $TCTMP/expect.scr <<-EOF
		#!$expcmd -f
		set env(USER) $temp_user
		set timeout 10
		set id \$env(USER)
		set RHOST \$env(HOST)
		proc abort {} { exit 1 }
		spawn passwd [lindex \$argv 0]
		set password [lindex \$argv 1]
		expect {
			timeout abort
			"password:" { sleep 2; send "\$password\r" }
		}
		expect {
			timeout abort
			"password:" { send "\$password\r" }
		}
		expect eof
		# spawn su and issues scp
		# to verify the password is set and correct
		spawn su - $temp_user
		expect {
			timeout abort
			"\$id@\$RHOST" { send "mkdir tmpd\r" }
		}
		expect { 
			timeout abort
			"\$id@\$RHOST" { send "touch aaa\r" }
		}
		expect { 
			timeout abort
			"\$id@\$RHOST" { send "scp aaa \$id@\$RHOST:./tmpd\r" }
		}
		expect {
			timeout abort
			"(yes/no)?" { send "yes\r" }
		}
		expect {
			timeout abort
			"password:" { send "\$password\r" }
		}
		expect {
			timeout abort
			"\$id@\$RHOST" { send "exit\r" }
		}
		expect eof
		#expect {
		#	timeout abort
		#	"\$id@\$RHOST"
		#}
		send_user "all done\n"
	EOF
	chmod +x $TCTMP/expect.scr
	$TCTMP/expect.scr $temp_user $password >$stdout 2>$stderr
	tc_pass_or_fail $? "expect file failed."
}

function test03()	# test puts, set, unset, send_error, info exists
{
	tc_register	"test set/unset/info exists"
	tc_exec_or_break cat chmod || return

	# create expect file to test Expect commands 
	local expcmd=`which expect`
	cat > $TCTMP/expect.scr <<-EOF
		#!$expcmd -f
		set sleep 99		;# set variable sleep to be 99
		
		# use catch to check if puts command fails
		if 1==[catch {puts "sleep is set to be \$sleep"}] {
			send_error "catch return 1 for puts command\n"
			exit 1
		}
		
		# test "info" command
		# "info exists" returns 1 if variable exists or 0 otherwise
		if 1==[info exists sleep] {
			send_user "ok, sleep is set!\n"
		} else {
			send_error "the variable - sleep - should be set\n"
			exit 1
		}	

		unset sleep		;# variable sleep can no longer be read

		if 1==[info exists sleep] {
			send_error "variable sleep should no longer be set\n"
			exit 1
		} else {
			send_user "ok, sleep is unset!\n"
		}	
		
		send_user "all done\n"
	EOF
	chmod +x $TCTMP/expect.scr
	$TCTMP/expect.scr >$stdout 2>$stderr
	tc_pass_or_fail $? "expect file failed." 
}

function test04()	# test proc, catch, and error 
{
	tc_register	"test proc/catch/error"
	tc_exec_or_break cat chmod || return

	# create expect file to test Expect commands 
	local expcmd=`which expect`
	cat > $TCTMP/expect.scr <<-EOF
		#!$expcmd -f
		set answer 6		;# set variable answer to be 6
		proc abort {} { exit 1 }
		proc sum {args} {
			set total 0
			foreach int \$args {
				if {\$int < 0} {error "number is < 0"}
				incr total \$int
			}
			return \$total
		}	
		
		# use catch to check if sum works as expected
		# catch returns 1 if there was an error or 0 if the
		# procedure returns normally
		# the return value from sum is saved in result
		if 0==[catch {sum 1 2 3} result] {
			send_user "catch return 0 for sum\n"
			if {\$result != \$answer} {
				send_error "error in sum\n"
				abort
			} 
		} else {
			send_error "catch return 1 for sum\n"
			abort
		}	

		# call proc sum again, but this time pass (-1, 2, 3) to it
		# on purpose to check if the error message is caught by result
		if 0==[catch {sum -1 2 3} result] {
			send_error "{sum -1 2 3} -> catch return 0\n"
			abort
		} else { 
			if {\$result == "number is < 0"} { 
				send_user "error is catched!\n"
			} else {
				send_error "error is not catched.\n"
				abort
			}	
		}	
		
		send_user "all done\n"
	EOF
	chmod +x $TCTMP/expect.scr
	$TCTMP/expect.scr >$stdout 2>$stderr
	tc_pass_or_fail $? "expect file failed." 
}

function test05()	# executing UNIX commands in Expect program  
{
	tc_register	"executing UNIX commands in Expect"
	tc_exec_or_break cat chmod || return 1

	# create expect file to executing UNIX program 
	local expcmd=`which expect`
	cat > $TCTMP/expect.scr <<-EOF
		#!$expcmd -f
		proc abort {} { exit 1 }
	
		if 1==[catch {exec touch $TCTMP/expfile}] {
			send_error "catch return 1 for touch command\n"
			abort
		}
		
		# use "file exists" to check if the file exists
		# return 1 if file exists, 0 otherwise
		if 0==[file exists $TCTMP/expfile] {
			send_error "file created with touch does not\
					exist\n"
			abort
		}

		if 1==[catch {exec rm $TCTMP/expfile}] {
			send_error "catch return 1 for rm command\n"
			abort
		}
		
		# use "file exists" to check if the file exists
		# return 1 if file exists, 0 otherwise
		if 1==[file exists $TCTMP/expfile] {
			send_error "error: file should be rm already, but\
					still exists\n"
			abort
		}	
		send_user "all done\n"
	EOF
	chmod +x $TCTMP/expect.scr
	$TCTMP/expect.scr >$stdout 2>$stderr
	tc_pass_or_fail $? "expect file failed." 
}


function test06()	# check if timeout actually takes effect  
{
	tc_register	"check if timeout actually takes effect"
	tc_exec_or_break cat chmod || return 1

	# create expect file to executing UNIX program 
	local expcmd=`which expect`
	cat > $TCTMP/expect.scr <<-EOF
		#!$expcmd -f
		set timeout 3
		spawn sleep 1000
		proc done {} { puts "timeout"; exit 0 }
	
		expect {
			timeout done
			"ERROR"
			}

		send_user "ERROR: should not see this!\n"
		exit 1
	EOF
	chmod +x $TCTMP/expect.scr
	$TCTMP/expect.scr >$stdout 2>$stderr
	tc_pass_or_fail $? "expect file failed." 
}

################################################################################
# main
################################################################################

TST_TOTAL=6

tc_setup
tc_root_or_break || exit

tc_add_user_or_break || exit

test01 &&
test02 &&
test03 &&
test04 &&
test05 &&
test06
