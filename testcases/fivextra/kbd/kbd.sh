#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		      ##
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
# File :	kbd.sh
#
# Description:	keyboard input driver project is composed of core programs and 
#		keymaps, the programs are loadkeys, dumpkeys, and showkey
#
# Author:	Helen Pang, hpang@us.ibm.com
#
# History:	May 16 2003 - Created. Helen Pang. hpang@us.ibm.com
#		May 27  2003 - Updates after code review.
#
#		16 Dec 2003 - (hpang) updated to tc_utils.source
################################################################################
# source the standard utility functions
###############################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# the testcase functions
################################################################################

#
# test01        loadkeys/dumpkeys (load/dump default keymap:\
#				   loadkeys -d or --default \
#				   dumpkeys)
#
function test01()
{
        tc_register "load/dump default keymap"

	# load default keymap
	loadkeys -d &>$stdout   # dumb loadkeys puts good output to stderr
	tc_fail_if_bad $? "failed to load default keymap" || return

	# dump default keymap 
	dumpkeys >$TCTMP/defkeymap 2>$stderr
	tc_fail_if_bad $? "Unexpected result" || return
	
	# check load default keymap
	grep 58 $TCTMP/defkeymap >$stdout 2>$stderr
	tc_fail_if_bad $? "failed to grep keycode number 58 from defkeymap" || return
	grep Caps_Lock $stdout >/dev/null 2>$stderr
	tc_fail_if_bad $? "failed to match Caps_Lock key to keycode number 58" || return

        grep 29 $TCTMP/defkeymap >$stdout 2>$stderr
        tc_fail_if_bad $? "failed to grep keycode number 29 from defkeymap" || return
        grep Control $stdout >/dev/null 2>$stderr
        tc_fail_if_bad $? "failed to match Control key to keycode number 29" || return

        tc_pass_or_fail 0 "Will Never Fail Here"
}

#
# test02	loadkeys (load/check specified keymap)
#
function test02()
{
	tc_register "load/check specified keymap"
	
	# load myfile as specified keymap
	dumpkeys | head -1 >$TCTMP/myfile   # gets default keymap region
	echo "keycode 58 = Control" >>$TCTMP/myfile
	echo "keycode 29 = Caps_Lock" >>$TCTMP/myfile
	loadkeys $TCTMP/myfile &>$stdout   #dumb loadkeys puts good output to stderr
	tc_fail_if_bad $? "failed to load myfile" || return

	# check load myfile as modify keymap
	dumpkeys >$TCTMP/my_keymap 2>$stderr
	tc_fail_if_bad $? " failed to dump my_keymap" || return
	grep 58 $TCTMP/my_keymap >$stdout 2>$stderr
	tc_fail_if_bad $? "failed to grep keycode number 58 from my_keymap" || return
	grep  Control $stdout >/dev/null 2>$stderr
	tc_fail_if_bad $? "failed to match Control key to keycode number 58" || return
	
	grep 29 $TCTMP/my_keymap >$stdout 2>$stderr
	tc_fail_if_bad $? "failed to grep keycode number 29 from my_keymap" || return
	grep Caps_Lock $stdout >/dev/null 2>$stderr
	tc_fail_if_bad $? "failed to match Caps_Lock key to keycode number 29" || return
	
	loadkeys -d &>$stdout	# dumb loadkeys puts good output to stderr
	tc_pass_or_fail $? "failed to load the default keymap"
}

#
# test03	loadkeys (load/check kernel string table to reach a\ 
#			   well-defined state: loadkeys -s)
#
function test03()
{
	tc_register "load/check kernel string table"

	# load string table
	dumpkeys | head -1 >$TCTMP/myfile   # gets default keymap region
	echo "keycode 63 = F70 F71" >>$TCTMP/myfile
	echo "string F70 = \"Hello\"" >>$TCTMP/myfile
	echo "string F71 = \"Good-Bye\"" >>$TCTMP/myfile
	loadkeys -s $TCTMP/myfile &>$stdout   #dumb loadkeys puts good output to stderr
	tc_fail_if_bad $? "Unexpected result" || return

	# check load string table
	dumpkeys >$TCTMP/my_keymap 2>$stderr
	tc_fail_if_bad $? " failed to dump my_keymap" || return
	sc=`grep string $TCTMP/my_keymap | wc -l`
	[ $sc == 2 ]
	tc_fail_if_bad $? "failed to load string table" || return
	
	loadkeys -d &>$stdout	# dumb loadkeys puts good output to stderr
	tc_pass_or_fail $? "failed to load the default keymap"
}

#
# test04	loadkeys (load/check kernel accent table to empty the \
#			   kernel accent table: loadkeys -c)
#
function test04()
{
	tc_register "load/check kernel accent table"
	
	# load accent table
	dumpkeys | head -1 >$TCTMP/myfile   # gets default keymap region
	echo "compose '-' 'Y' to '¥'" >>$TCTMP/myfile
	loadkeys -c $TCTMP/myfile &>$stdout   #dumb loadkeys puts good output to stderr
	tc_fail_if_bad $? "Unexpected result" || return

	# check accent table
	dumpkeys >$TCTMP/my_keymap 2>$stderr
	tc_fail_if_bad $? " failed to dump my_keymap" || return
	cc=`grep compose $TCTMP/my_keymap | wc -l`
	[ $cc == 1 ]
	tc_fail_if_bad $? "failed to load accent table" || return
	
	loadkeys -d &>$stdout	# dumb loadkeys puts good output to stderr
	tc_pass_or_fail $? "failed to load the default keymap"
}

#
# test05	dumpkeys (dump/check key bindings: dumpkeys --keys-only)	
#
function test05()
{
	tc_register "dump/check keys-only"

	# dump keymap w/ key bindings only
	dumpkeys --keys-only >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected result" || return

	# check dump key bindings
	grep -v string $stdout >/dev/null 2>$stderr
	tc_pass_or_fail $? "failed to dump the key bindings"
}

#
# test06	dumpkeys (dump/check string definitions : dumpkeys --funcs-only)
#
function test06()
{
	tc_register "dump/check funcs-only"

	# dump keymap w/ string definitions only
	dumpkeys --funcs-only >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected result" || return

	# check dump string definitions
	grep string $stdout >/dev/null 2>$stderr
	tc_pass_or_fail $? "failed to dump the string definitions"
}

#
# test07	dumpkeys (dump/check compose key combinations: \
#			  dumpkeys --compose-only)
#
function test07()
{
	tc_register "dump/check compose-only"
	
	# dump keymap w/ compose key combinations only
	dumpkeys --compose-only >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected result" || return

	# check dump compose key combinations
	grep -v compose $stdout >/dev/null 2>$stderr
	[ $? -ne 0 ]
	tc_pass_or_fail $? "failed to dump the compose key combinations"
}

#
# test08	dumpkeys (dump/check short/long info listings: \
#			  dumpkeys -i/-l)
#
function test08()
{
	tc_register "dump/check short/long listings"

	# dump keymap w/ short info listings
	dumpkeys -i >$TCTMP/short 2>$stderr
	tc_fail_if_bad $? "Unexpected result" || return

	# dump keymap w/ long info listings"
	dumpkeys -l >$TCTMP/long 2>$stderr
	tc_fail_if_bad $? "Unexpected result" || return

	# check dump short/long info listings
	slc=`cat $TCTMP/short | wc -l`
	llc=`cat $TCTMP/long | wc -l`
	[ $slc -lt $llc ]
	tc_pass_or_fail $? "failed to dump short/long info listings"
}

#
# test09        showkey (print/check scancodes to standard output: showkey -s)
#
function test09()
{
        tc_register "print/check scancodes"

        # set print scancode from keypress/release to tty
        tc_info "take 10s to complete the test"
        showkey -s >$stdout 2>$stderr
        tc_fail_if_bad $? "Unexpected result" || return

        # check set print scancodes from keypress/release to tty
        grep keycode $stdout >/dev/null 2>$stderr
	[ $? -ne 0 ]
        tc_fail_if_bad $? "failed print scancodes to tty" || return

        tc_pass_or_fail 0 "Will Never Fail Here"
}

#
# test10        showkey (print/check keycodes to standard output: showkey -k)
#
function test10()
{
        tc_register "print/check keycodes"

        # set print keycodes from keypress/release to tty
        tc_info "take 10s to complete the test"
        showkey -k >$stdout 2>$stderr 
        tc_fail_if_bad $? "Unexpected result" || return

        # check set print keycodes from keypress/release to tty
        grep 0x $stdout >/dev/null 2>$stderr
        [ $? -ne 0 ]
        tc_fail_if_bad $? "failed print keycode to tty" || return

        tc_pass_or_fail 0 "Will Never Fail Here"
}

################################################################################
# main
################################################################################

TST_TOTAL=10

# standard tc_setup
tc_setup

tc_root_or_break || exit

test01 &&\
test02 &&\
test03 &&\
test04 &&\
test05 &&\
test06 &&\
test07 &&\
test08 &&\
test09 &&\
test10
