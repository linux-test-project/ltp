#!/bin/sh
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
# File :	pidentd.sh
#
# Description:	Tests the functionality of pidentd.
#
# Author:	Andrew Pham, apham@us.ibm.com
#
# History:	April 01 2003 - Created. Andrew Pham, apham@us.ibm.com
#		April 18 2003 - Verify that ikeygen and idecrypt exist
#				before testing - Andrew Pham
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
################################################################################
commands="rcidentd ibench "

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################
REQUIRED="ps grep ls echo"
IDENTD_ON=0
FIREWA_ON=0
CONF_C=0
TST_TOTAL=2
ErrMsg="not available."
ErrMsg1="Unexpected output:"
################################################################################
# utility functions
################################################################################
function tc_local_setup()
{
	tc_root_or_break || exit

	# make sure the firewall is not running.
	ps -ax >$TCTMP/ps_info 2>/dev/null
	if grep firewall $TCTMP/ps_info >&/dev/null ; then 
		tc_info "Please turn off the firewall."
		FIREWA_ON=1
	fi

	# make sure the identd started up as a standalone server.
	if [ -s /etc/inetd.conf ] && grep identd /etc/inetd.conf &>/dev/null ; then
		tc_info "Please edit /etc/inetd.conf so identd can be"
		tc_info "started as a standalone server, e.g. comment"
		tc_info "out the line for in.identd."
		CONF_C=1
		/etc/init.d/identd status | grep running >&/dev/null
		IDENTD_ON=$?
	fi
}

function tc_local_leanup()
{
	if [ $CONF_C -eq 1 ]; then
		tc_info "You might need to restore /etc/inetd.conf."
	fi
	if [ $FIREWA_ON -eq 1 ]; then
		tc_info "You might need to restart the firewall."
	fi
	if [ $IDENTD_ON -eq 0 ]; then
		/etc/init.d/identd stop >&/dev/null
	else
		/etc/init.d/identd start >&/dev/null
	fi
}
	
################################################################################
# the testcase functions
################################################################################
function TC_rcidentd()
{
	TCNAME="rcidentd stop"
	rcidentd stop >$stdout 2>$stderr
	tc_fail_if_bad $? "$ErrMsg." || return

	ps -xa >$stdout 2>/dev/null
	grep -v identd $stdout >&/dev/null
	tc_pass_or_fail $? "$ErrMsg1" || return
	
	tc_register "rcidentd start"
	let TST_TOTAL+=1
	
	rcidentd start >$stdout 2>$stderr
	tc_fail_if_bad $? "$ErrMsg." || return

	ps -xa >$stdout 2>/dev/null
	grep identd $stdout >&/dev/null
	tc_pass_or_fail $? "$ErrMsg1"
}

function TC_ibench()
{
	# make sure identd server is running.
	/etc/init.d/identd restart >& /dev/null && {
		tc_break_if_bad $? "Unable to restart identd.";
		return ; }

	tc_info "Start testing ibench: please wait."
	ibench 2>$stderr >$stdout
	tc_pass_or_fail $? "Stress test failed." || return
	
	tc_info "Please refer to the manual instructions to do "
	tc_info "the manual test part."
}

function TC_ikeygen()
{
	ikeygen >$stdout 2>$stderr
	tc_fail_if_bad $? "$ErrMsg." || return

	[ -s /etc/identd.key ]
	tc_pass_or_fail $? "$ErrMsg1"
}

function TC_idecrypt()
{
	echo "123abc" >$TCTMP/decry 2>$stderr

	idecrypt $TCTMP/decry >$stdout 2>$stderr
	tc_pass_or_fail $? "Failed."
}
################################################################################
# main
################################################################################
tc_setup

# Check for supporting utilities
tc_exec_or_break $REQUIRED || return

FRC=0
for cmd in $commands
do
	tc_register "$cmd" 
	TC_$cmd || FRC=$?
done
if type ikeygen >&/dev/null; then
	tc_register "ikeygen"
	        TC_ikeygen || FRC=$?
fi

if type idecrypt >&/dev/null; then
        tc_register "idecrypt"
	TC_idecrypt || FRC=$?
fi
exit $FRC		
