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
## WITHOUT ANY WARRANTY; without even the implied warranty of MEECHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	telnet.sh
#
# Description:	Test telnet package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
# History:	April 02 2003 - Created - Andrew Pham
#
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
#		05 May 2004 - (rcp) fix bug where expect sends password.
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TST_TOTAL=3
REQUIRED="telnet in.telnetd chmod grep cat rm expect touch which ps"

my_port=122

################################################################################
# utility functions
################################################################################

function start_telnet_daemon()
{
	netstat -lpen | grep -q "$my_port" && {
		# in.telnetd already running, killit 
		killall in.telnetd >& /dev/null
	}

	in.telnetd -debug $my_port &
	tc_break_if_bad $? "could not start in.telnetd"
	local counter=0
	while true ; do
		let counter+=1
		[ $counter -lt 10 ]
		tc_fail_if_bad $? "could not start telnet daemon" || return
		netstat -lpen | grep -q "$my_port" && break
		echo "($i) waiting for in.telnetd to start ..."
		sleep 1
	done
}

function tc_local_setup()
{
	tc_add_user_or_break || return
}

function tc_local_cleanup()
{
	# kill telnetd
	killall in.telnetd >& /dev/null
}
################################################################################
# testcase functions
################################################################################
function TC_telnet1()
{	
	start_telnet_daemon || return

	# Create expect scritp
	local expcmd=`which expect`
	cat > $TCTMP/mtelnet <<-EOF
	#!$expcmd -f
	set timeout 10
	set id $temp_user
	set tfile [lindex \$argv 0]
	set tport [lindex \$argv 1]
	proc abort {} { exit 1 }
	spawn telnet localhost \$tport
	expect {
		timeout abort
		"login:" { send "\$id\r" }
	}
	expect {
		timeout abort
		"Password:" { sleep 2; send "password\r" }
	}
	expect {
		timeout abort
		"\$id@" { send "touch \$tfile\r" }
	}
	expect {
		timeout abort
			"\$id@\$" { send "exit\r" }
		}
	expect eof
	EOF
	chmod +x $TCTMP/mtelnet >&/dev/null

	local TFILE="/tmp/my$$testfile"
	$TCTMP/mtelnet $TFILE $my_port >&/dev/null
	netstat -lpen | grep -q $my_port && started="yes"

	if [ ! -e $TFILE ]; then
		tc_pass_or_fail 1 "telnet is not working as expected."
		return 1
	fi
	tc_pass_or_fail 0 "passed"
	rm $TFILE

	netstat -lpen | grep -q $my_port && started="yes"
}

function TC_telnet2()
{	
	start_telnet_daemon || return

	# Create expect scritp
	local expcmd=`which expect`
	cat > $TCTMP/mtelnet2 <<-EOF
	#!$expcmd -f
	set timeout 10
	set id $temp_user
	set tfile [lindex \$argv 0]
	set tport [lindex \$argv 1]
	proc abort {} { exit 1 }
	spawn telnet -a -l $temp_user localhost \$tport
	expect {
		timeout abort
		"Password:" { sleep 2; send "password\r" }
	}
	expect {
		timeout abort
		"\$id@" { send "touch \$tfile\r" }
	}
	expect {
		timeout abort
			"\$id@" { send "exit\r" }
		}
	expect eof
	EOF
	chmod +x $TCTMP/mtelnet2 >&/dev/null

	local TFILE="/tmp/my$$testfile2"
	$TCTMP/mtelnet2 $TFILE $my_port >$stdout 2>$stderr
	
	if [ ! -e $TFILE ]; then
		tc_pass_or_fail 1 "telnet is not working as expected."
		return 1
	fi
	tc_pass_or_fail 0 "passed"

	rm $TFIL$TCTMP/mtelnet2 $TFILE
	return 0
}

function TC_telnet3()
{	
	start_telnet_daemon || return

	# Create an expect script
	local expcmd=`which expect`
	cat > $TCTMP/mtelnet3 <<-EOF
	#!$expcmd -f
	set timeout 10
	set id $temp_user
	set lfile [lindex \$argv 0]
	set tport [lindex \$argv 1]
	
	proc abort {} { exit 1 }
	spawn telnet localhost \$tport
	log_file -noappend -a \$lfile
	
	expect {
		timeout abort
		"login:" { send "\$id\r" }
	}
	expect {
		timeout abort
		"Password:" { sleep 2; send "password\r" }
	}
	expect {
		timeout abort
			"\$id@" { send "exit\r" }
		}
	expect eof
	EOF

	# Execute the expect script
	chmod +x $TCTMP/mtelnet3 >&/dev/null
	$TCTMP/mtelnet3 $stdout >&/dev/null
	grep -v '[Ww]elcome' $stdout >&/dev/null
	tc_pass_or_fail $? "Unexpected output."  
}

################################################################################
# main
################################################################################
tc_setup

# Check if supporting utilities are available
tc_exec_or_break  $REQUIRED || exit

E_value=0
tc_register "telnet"
TC_telnet1 || E_value=1
tc_register "telnet -a -l"
TC_telnet2 || E_value=1
tc_register "telnetd -h"
TC_telnet3 || E_value=1
exit $E_value
