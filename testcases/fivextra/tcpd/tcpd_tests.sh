#!/bin/bash
################################################################################
#                                                                              #
#  (C) Copyright IBM Corp. 2003                                                #
#                                                                              #
#  This program is free software;  you can redistribute it and/or modify       #
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
# File :       tcpd_tests.sh
#
# Description: This program tests basic functionality of tcpd (tcp wrappers)
#
# Author:      Manoj Iyer  manjo@mail.utexas.edu
#
# History:     Apr 17 2003 - Created - Manoj Iyer
#              July 29 2003 - Modified - Manoj Iyer
#              - Heavily modified inorder to standardize.
#              Aug 21 2003 - fixed - check for correct error code from 
#                            do_ssh.tcl program.
#              Oct 20 2003 - modify to use local_setup - Robb Romans
#		08 Jan 2004 - (RR) updated to tc_utils.source

# Global definitions.

# error type returned by the do_ssh.tcl
SSH_SUCCESS=0      # pam authentication success
TCLUSE_ERR=1       # expect program usage error
TIME_OUT=2         # expect did not send passwd, timed out.
SPAWN_ERR=3        # expect failed to spawn pam_authuser program

# array contains the error messages indexed by error type 
# retuned by the pam_loguser.tcl program.
errmsg[SSH_SUCCESS]="ssh to user $(whoami) success"
errmsg[TCLUSE_ERR]="expect program (do_ssh.tcl) usage error"
errmsg[TIME_OUT]="ssh $(whoami)@localhost refused connection."
errmsg[SPAWN_ERR]="expect failed to spawn ssh program"


# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# test setup functions
tc_local_setup() {

	if [ -f /etc/hosts.deny ] ; then
		cp /etc/hosts.deny /etc/hosts.deny.orig
	else {
		tst_resm TBROK "No hosts.deny file!"
		exit 3
		}
	fi
}

tc_local_cleanup() {

	if [ -f /etc/hosts.deny.orig ] ; then
		cp /etc/hosts.deny.orig /etc/hosts.deny
	fi
}

#
# Function: test01
#
# Description: This testcase tests the basic functionality of tcpd
#              - deny ssh access to ALL add sshd : ALL to hosts.deny
#              - call utility that does ssh, this should fail
#              - remove from deny list, the entry  sshd : ALL 
#              - utility should return authentication success.
# 
# Inputs:      NONE
#
# Exit:        0        - on success.
#              non-zero - on failure.
#
test01()
{

	tc_register "tcpd functionality"

        # modify hosts.deny file to add ssh
	echo "sshd : ALL" > /etc/hosts.deny >$stdout 2>stderr
	tc_fail_if_bad $? "$TCNAME: failed to add sshd to deny list" || return
	
        # call utility to ssh to $(whoami)@localhost
	do_ssh.tcl $(whoami)@localhost ~. >$stdout 2>$stderr
	[ $? -ne 2 ] && { tc_fail_if_bad $? ${errmsg[$?]} || return ; }
    
	# repeat ssh after restoring
	do_ssh.tcl $(whoami)@localhost ~. >$stdout 2>$stderr
	tc_pass_or_fail $? ${errmsg[$?]}		
}


# Function: main
# 
# Description: - call setup function.
#              - execute each test.
#
# Inputs:      NONE
#
# Exit:        zero - success
#              non_zero - failure
#
TST_TOTAL=1
tc_setup
tc_root_or_break || exit
tc_exec_or_break cp ssh expect whoami || exit

test01
