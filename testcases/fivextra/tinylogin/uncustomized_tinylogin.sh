#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab:
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2001		      ##
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
# File :	tinylogin.sh
#
# Description:	Test the functions provided by tinylogin.
#
# Dependency:	This requires the tinylogin.links file created by busybox's
#		Makefile. Copy it to this directory.
#
# Hint:		Search for "main" to see where execution starts
#
# Author:	Andrew Pham, apham@us.ibm.com
#
# History:	Mar 21 2003 - Created. Andrew Pham, apham@us.ibm.com
#		May 06 2003 - Added the rest of the test cases for tinylogin.
#		17 Nov 2003 - (RCP) add "sleep 2" to avoid timing problem with
#				expect and passwd. BUG 5203.
#
#		17 Nov 2003 (rcp) updated to tc_utils.source

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

# NOTE:	"[P]UTNAMESHERE" is replaced with a list of supported tinylogin functions
#	by the "insert_names.sh" script which is run by "make all"
names="PUTNAMESHERE" # tinylogin commands to be tested

REQUIRED="tinylogin cat cut grep"


TL=tinylogin	# the tinylogin executable.

ErrMsg="Failed: Not available."
ErrMsg1="Failed: Unexpected output.  Expected:"
################################################################################
# local utility functions
################################################################################
function uniq_name()
{
	let u_ndx+=1
	local nn="00$u_ndx"
	local -i off=-2 
	nn=${nn:off:2}   
	local u="$$00000" 
	u=${u:0:6}$nn  
	X_NAME=$u
}

function uniq_user()
{
	local RC=0
	uniq_name
	new_user="u$X_NAME"
	grep $new_user /etc/passwd >&/dev/null
	RC=$?
	while [ $RC -eq 0 ] 
	do
		uniq_name
		new_user="u$X_NAME"
		grep $new_user /etc/passwd >&/dev/null
		RC=$?
	done
}

function uniq_group()
{
	local RC=0
	uniq_name
	new_grp="g$X_NAME"
	grep $new_grp /etc/group >&/dev/null
	RC=$?
	while [ $RC -eq 0 ] 
	do
		uniq_name
		new_grp="g$X_NAME"
		grep $new_grp /etc/group >&/dev/null
		RC=$?
	done
}
################################################################################
# the testcase functions
################################################################################
function TC_addgroup()
{
	uniq_group
	$TL addgroup $new_grp >/dev/null 2>$stderr
	tc_fail_if_bad $? "$ErrMsg" || return
		
	grep $new_grp /etc/group >&/dev/null
	tc_pass_or_fail $? "$ErrMsg1 group $new_grp added."
	return 
}

function TC_delgroup()
{
	local RC=0

	$TL delgroup $new_grp >$stdout 2>$stderr
	tc_fail_if_bad $? "$ErrMsg" 
	RC=$?
		
	grep $new_grp /etc/group >&/dev/null
	[ $? -ne 0 ]
	tc_pass_or_fail $? "Group $new_grp is still there."
	RC=$?

#	[ $RC -ne 0 ] || groupdel $new_grp >&/dev/null
	return $RC 
}
function TC_adduser()
{
	local password="nobody1"
	# Check if supporting utilities are available
	tc_exec_or_break expect chmod || return

	# create an expect file
	local expcmd=`which expect`
	cat > $TCTMP/exp_script <<-EOF
		#!$expcmd -f
		proc abort {} { exit 1 }
		set timeout 10
		spawn tinylogin [lindex \$argv 0] [lindex \$argv 1]
		set password [lindex \$argv 2]
		expect {
			timeout abort
			"password:" { send "\$password\r" }
		}
		expect {
			timeout abort
			"password:" { send "\$password\r" }
		}
		expect eof
	EOF

	chmod +x $TCTMP/exp_script
	uniq_user
	$TCTMP/exp_script adduser $new_user $password >&/dev/null

	grep $new_user /etc/passwd >&/dev/null
	tc_pass_or_fail $? "$ErrMsg1 user: $new_user is not added."
	return
}
function TC_deluser()
{
	$TL deluser $new_user >/dev/null 2>$stderr
	tc_fail_if_bad $? "$ErrMsg"
	RC=$?

	grep $new_user /etc/passwd >&/dev/null
	[ $? -ne 0 ]
	tc_pass_or_fail $? "$ErrMsg1 user $new_user deleted."
	RC=$?

	[ $RC -ne 0 ] || userdel $new_user >&/dev/null
	return $RC
}
function TC_su()
{
	tc_info "su: Can only test if this is used as std su." 
	return 0

	# Check if supporting utilities are available
	tc_exec_or_break  touch mkdir || return 1

	# create an expect script 
	local expcmd=`which expect`
	cat > $TCTMP/su_exp_spt <<-EOF
		#!$expcmd -f
		proc abort {} { exit 1 }
		set env(USER) $temp_user
		set timeout 10
		set id \$env(USER)
		set RHOST \$env(HOST)
		spawn tinylogin [lindex \$argv 0] [lindex \$argv 1]
		expect {
			timeout abort
			"\$id@\$RHOST" { send "mkdir /tmp/su_tst\r" }
		}
		expect { 
			timeout abort
			"\$id@\$RHOST" { send "touch /tmp/su_tst/a\r" }
		}
		expect {
			timeout abort
			"\$id@\$RHOST" { send "exit\r" }
		}
		expect eof
	EOF
		
	chmod +x $TCTMP/su_exp_spt
	$TCTMP/su_exp_spt su $temp_user #>&/dev/null
	
	[ -e /tmp/su_tst/a ]
	tc_pass_or_fail $? "$ErrMsg1" || return 1

	rm -rf /tmp/su_tst >&/dev/null
	return 0
}
function TC_login()
{
	tc_info "login: Can only test this if it is uses as std login."
	return 0

	# Check if supporting utilities are available
	tc_exec_or_break  cat passwd touch mkdir || return 1

	# create an expect script 
	local expcmd=`which expect`
	cat > $TCTMP/pw_exp_spt <<-EOF
		#!$expcmd -f
		set timeout 10
		proc abort {} { exit 1 }
		spawn passwd [lindex \$argv 0]
		expect {
			timeout abort
			"password:" { sleep 2; send "password\r" }
		}
		expect {
			timeout abort
			"password:" { send "password\r" }
		}
		expect eof
	EOF

	chmod +x $TCTMP/pw_exp_spt
	$TCTMP/pw_exp_spt $temp_user >&/dev/null
	
	cat > $TCTMP/login_exp_spt <<-EOF
		#!$expcmd -f
		proc abort {} { exit 1 }
		set env(USER) $temp_user
		set timeout 10
		set id \$env(USER)
		set RHOST \$env(HOST)
		spawn tinylogin [lindex \$argv 0] [lindex \$argv 1]
		expect {
			timeout abort
			"Password:" { send "password\r" }
		}
		expect {
			timeout abort
			"\$id@\$RHOST" { send "mkdir /tmp/login_tst\r" }
		}
		expect { 
			timeout abort
			"\$id@\$RHOST" { send "touch /tmp/login_tst/a\r" }
		}
		expect {
			timeout abort
			"\$id@\$RHOST" { send "exit\r" }
		}
		expect eof
	EOF
		
	chmod +x $TCTMP/login_exp_spt
	$TCTMP/login_exp_spt login $temp_user >&/dev/null
	
	[ -e /tmp/login_tst/a ]
	tc_pass_or_fail $? "$ErrMsg1 /tmp/login_tst/a to exist." || return 1

	rm -rf /tmp/login_tst >&/dev/null
	return 0
}
function TC_passwd()
{
	local OldPwd=`grep $new_user /etc/shadow | cut -f2 -d:`
	# create an expect script 
	local expcmd=`which expect`
	cat > $TCTMP/passwd_exp_spt <<-EOF
		#!$expcmd -f
		set timeout 10
		proc abort {} { exit 1 }
		spawn tinylogin passwd [lindex \$argv 0]
		expect {
			timeout abort
			"new password:" { send "test\r" }
		}
		expect {
			timeout abort
			"new password:" { send "test\r" }
		}
		expect eof
	EOF

	chmod +x $TCTMP/passwd_exp_spt
	$TCTMP/passwd_exp_spt $new_user >&/dev/null
	
	local NewPwd=`grep $new_user /etc/shadow | cut -f2 -d:`

	[ "$NewPwd" != "$OldPwd" ]
	tc_pass_or_fail $? "Unexpected output: Password did not change $NewPwd=$OldPwd."
	return
}
function TC_getty()
{
	tc_info "getty is not tested"
	return 0
}
################################################################################
# main
################################################################################

TST_TOTAL=0
set `echo $names`; TST_TOTAL=$#

tc_setup

tc_root_or_break || exit

# run tests against all supported tinylogin applets
for TCNAME in $names ; do
	tc_register $TCNAME
	TC_$TCNAME || exit
done
