#!/bin/bash
################################################################################
#                                                                              #
#  (C) Copyright IBM Corp. 2003                                                #
#                                                                              #
#  This program is free software;  you can redistribute it and#or modify       #
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
# File :	   omshell_tests.sh
#
# Description: This program tests basic functionality of omshell command.
#
# Author:	   Manoj Iyer  manjo@mail.utexas.edu
#
# History:	   Sept 02 2003 - created - Manoj Iyer
#		08 Jan 2004 - (RR) updated to tc_utils.source


# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source


#
# Function:    test01
#
# Description:	- Test that omshell with command 'help' will display the
#			help menu.
#		- execute command omshell with help and look for keywords like
#			server conect etc.
#
# Inputs:	   NONE
#
# Exit		   0 - on success
#		   non-zero on failure.
test01()
{
	tc_register    "omshell help functionality"
	
	tc_info "executing command omshell to display help menu."
	echo "help" | omshell 1>$TCTMP/tst_omshell.out 2>$stderr
	tc_fail_if_bad $? "omshell failed to start"

	# create the expected menu output omshell help.
	cat <<-EOF >$TCTMP/tst_omshell.exp 
		> Commands:
		port <server omapi port>
		server <server address>
		key <key name> <key value>
		connect
		new <object-type>
		set <name> = <value>
		create
		open
		update
		unset <name>
		refresh
		remove
		>
	EOF

	diff -qiwB $TCTMP/tst_omshell.exp $TCTMP/tst_omshell.out 2>$stderr 1>$stdout
	tc_pass_or_fail $? "failed to produce expected output."
}


#
# Function:    test02
#
# Description: - Test that omshell with command 'server 127.0.0.1' will
#				 connect to the omshells default server
#
# Inputs:	   NONE
#
# Exit		   0 - on success
#			   non-zero on failure.
test02()
{
	tc_register    "omshell server functionality"
	
	tc_info "omshell to connect to default server 127.0.0.1."
	echo "server 127.0.0.1" | omshell 1>$TCTMP/tst_omshell.out 2>$stderr
	tc_pass_or_fail $? "omshell failed to start"
}


# Function: main
# 
# Description: - call setup function.
#			   - execute each test.
#
# Inputs:	   NONE
#
# Exit:		   zero - success
#			   non_zero - failure
#

TST_TOTAL=2
tc_setup
#tc_root_or_break || exit
tc_exec_or_break omshell diff || exit

test01 &&
test02
