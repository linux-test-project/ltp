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
# File :   fbset_tests.sh
#
# Description: This program tests basic functionality of fbset command.
#
# Author:   Manoj Iyer  manjo@mail.utexas.edu
#
# History:	Aug 13 2003 - created - Manoj Iyer
#		Oct 14 2003 - modified - Manoj Iyer
#			    - check for X on system
#			    - use local setup and cleanup
#		Oct 29 2003 - modified - Andrew Pham
#			    - minor tweaks to get the tc to execute on mpc2
#			      before it just reported BROK
#		08 Jan 2004 - (RR) updated to tc_utils.source, cleanup
#		29 Jan 2004 (rcp) removed load of vga16fb module -- kernel
#				will autoload it. In some cases support is
#				not a module.
#		27 Jul 2004 (rcp) don't look fpor too many things in stdout.
#				Also, put an info message about expected failure.
#		02 Aug 2004 - modified - Gong Jie <gongjie@cn.ibm.com>
#			    - testcase start and shutdown Xvfb automatically.

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

#
# tc_local_cleanup
#
tc_local_cleanup()
{
	[ "$XRES" = "NONE" ] ||
	{
		fbset -xres $XRES -yres $YRES >$stdout 2>$stderr || \
		tc_break_if_bad $? "unable to restore original geometry" ||
			return
		tc_info "restored ${XRES}x$YRES geometry"
	}
	killall -9 Xvfb
}

#
# tc_local_setup
#
tc_local_setup()
{
	XRES="NONE"
	YRES="NONE"

	tc_root_or_break || return
	tc_exec_or_break grep Xvfb || return

	( Xvfb :6 -screen scrn 1024x768x16 & ) &>/dev/null
}

#
# test01	installation check
#
function test01()
{
	tc_register "installation check"
	tc_executes fbset
	tc_fail_if_bad $? "fbset not properly installed"

	fbset >$stdout 2>$stderr
	tc_pass_or_fail $? "unable to get current fb settings" || return

	set $(grep geometry $stdout)
	XRES=$2
	YRES=$3
	
	tc_info "Original geometry is ${XRES}x$YRES"
}

#
# test02	Test that fbset with no input parameters will display the 
#		current frame buffer settings.
#
function test02()
{
	tc_register    "fbset displays correct output"

	fbset >$stdout 2>$stderr
	tc_fail_if_bad $? "failed to print default settings" || return

	grep -q "geometry" $stdout &&
	grep -q "timings" $stdout &&
	grep -q "rgba" $stdout &&
	grep -q "Hz" $stdout
	tc_pass_or_fail $? "expected key words not found"
}

#
# test03	Test that fbset command can resize the geometry
#
function test03()
{
	tc_register    "fbset geometry to 640x480"

	tc_info "This test seems to fail, even on SuSE. Probably not a valid testcase."
	
	fbset -xres 640 -yres 480 2>$stderr 1>$stdout
	tc_fail_if_bad $? "Failed to to set resolution to 640x480" || return

	fbset >$stdout 2>$stderr
	grep -q "640x480" $stdout 2>>$stderr
	tc_pass_or_fail $? "Failed to set 640x480 (or failed to report it)"
}

# 
# main
# 

TST_TOTAL=3

tc_setup
test01 &&
test02 &&
test03
