#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2004						      ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MEECHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :        sudo_test.sh
#
# Description:  Test sudo command
#
# Author:      	CSDL  James He <hejianj@cn.ibm.com>
#		CSDL  	       <lizhig@cn.ibm.com>	
# History:      12 May 2004 created.
#		19 May 2004 Add sudotest06 sudotest07
#		26 May 2004 Rewrite test case.
#
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

CONTENT1="user1's temporary file content"
CONTENT2="user2's temporary file content"

################################################################################
# environment functions
################################################################################

#
# local setup
#
function tc_local_setup()
{
	tc_exec_or_break sudo hwclock echo cat grep cp || exit
	tc_root_or_break || exit

#add sudo test users
        tc_add_user_or_break sudotestadmin || exit
        USER1=$temp_user
        tc_add_user_or_break sudotestuser || exit
        USER2=$temp_user

	[ -e /etc/sudoers ] && mv /etc/sudoers $TCTMP/sudoers
        cat > /etc/sudoers <<-EOF
	User_Alias      SYSTEM_ADMIN = $USER1
	User_Alias      SYSTEM_USER = $USER2
	Cmnd_Alias      SU = /bin/su
	Cmnd_Alias      HWCK = /sbin/hwclock
	Cmnd_Alias      URCP = /bin/cp
	Cmnd_Alias      SUCK = /sbin/sudotest
	Defaults        syslog=auth,log_year,logfile=/var/log/sudo.log
	root    ALL=(ALL) ALL
	SYSTEM_ADMIN    ALL = NOPASSWD:ALL
	SYSTEM_USER     ALL = NOPASSWD:HWCK,URCP
	SYSTEM_USER     ALL = SUCK,!SU
	EOF
	chmod 0640 /etc/sudoers
        cp -f ./sudotest /sbin/
	tc_break_if_bad $? "failed to copy sudotest file " || exit

}

#
# local cleanup
#
function tc_local_cleanup
{
	[ -e $TCTMP/sudoers ] && mv $TCTMP/sudoers /etc/sudoers
	[ -e /sbin/sudotest ] && rm -f /sbin/sudotest
}

################################################################################
# testcase functions
################################################################################

function sudotest01
{
	COMMAND="sudo /sbin/hwclock"
	tc_register	"$USER1 $COMMAND"
	su - $USER1 -c "$COMMAND" >$stdout 2>$stderr
	tc_pass_or_fail $? "$USER1 can no run hwclock as root!"
}

function sudotest02
{
	COMMAND="sudo /sbin/hwclock"
	tc_register	"$USER2 $COMMAND"
	su - $USER2 -c "$COMMAND" >$stdout 2>$stderr
	tc_pass_or_fail $? "$USER2 can no run hwclock as root!"
}

function sudotest03
{
	COMMAND="sudo /sbin/sudotest"
	tc_register	"$USER1 $COMMAND"
	su - $USER1 -c "$COMMAND" >$stdout 2>$stderr
	tc_pass_or_fail $? "$USER1 can no run sudotest as root!"
}

function sudotest04
{
	COMMAND="sudo -S /sbin/sudotest"
	tc_register	"$USER2 $COMMAND"
	su - $USER2 -c "echo password | $COMMAND" >$stdout 2>&1
	tc_pass_or_fail $? "$USER2 can no run sudotest as root!"
	
}

function sudotest05
{
	COMMAND="sudo -S /bin/su"
	tc_register	"$USER2 $COMMAND"
	su - $USER2 -c "echo password | $COMMAND"  >$stdout 2>&1
	tc_pass_or_fail !$? "$USER2 can run su as root!"
	
}

function sudotest06
{
	COMMAND="cp -f"
	tc_register	"$USER1 $COMMAND"
        echo -n "$CONTENT1" > /home/$USER1/tmpfile
        chown $USER1 /home/$USER1/tmpfile
	sudo -u $USER1 $COMMAND /home/$USER1/tmpfile /home/$USER1/bakfile >$stdout 2>$stderr
	[ -e /home/$USER1/bakfile ]
        tc_pass_or_fail $? "can not copy $USER1's file from /home/$USER1/tmpfile to /home/$USER1/bakfile"
	
}

function sudotest07
{
	COMMAND="cp -f"
	tc_register	"$USER2 $COMMAND"
        echo -n "$CONTENT2" > /home/$USER2/tmpfile
        chown $USER2 /home/$USER2/tmpfile
	sudo -u $USER2 $COMMAND /home/$USER2/tmpfile /home/$USER2/bakfile >$stdout 2>$stderr
	[ -e /home/$USER2/bakfile ]
        tc_pass_or_fail $? "can not copy $USER2's file from /home/$USER2/tmpfile to /home/$USER2/bakfile"
	
}

################################################################################
# MAIN
################################################################################
TST_TOTAL=7
tc_setup
sudotest01 &&
sudotest02 &&
sudotest03 &&
sudotest04 &&
sudotest05 &&
sudotest06 &&
sudotest07                                                                                                                                                           