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
# File :	libungif.sh
#
# Description:	Test libungif package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
#   This test assumes the gif_lib utilities are available from one of the
# path directories, and that GIF_DIR is set 
# to the directory holding these gif files:
# 1. solid2.gif
# 2. solid3.gif
#
# Gershon Elber, Feb 90.  Rewritten by Eric Raymond, December 1995.
# Revised by Toshio Kuratomi December 1998.
#
# History:	Sept 02 2003 - ported tests from
#		  libungif package to LTP - Andrew Pham
#		Nov 04 2003 - the gif2x11 utility used by the testcase is 
#		  hardcoded to use pixmap depth 8 thus when X server is 
#		  configured to use other depth (ie. 16) the utility fails.  
#		  I change the testcase to use xloadimage instead.
#			Andrew Pham
#		07 Jan 2004 - (apham) updated to tc_utils.source
#		13 Jan 2004 - (rcp) use gif2x11 if xloadimage not available
#		04 Aug 2004 - (james) use Xfbd as the X server. 
#
################################################################################
# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

PROGS="gifbg gifcomb grep gifhisto gifflip gifrsize gifpos gifclip gifrotat gifspnge"
REQUIRED="ps grep killall X sleep"
GIF_DIR=$LTPBIN
GIF_DISPLAY=""
################################################################################
# Utility functions
################################################################################
function mXon()		# check if X server is running
{
#	[ -e /tmp/.X*-lock ]
#		tc_break_if_bad	$? "The X server is not running." || return

# 	set DISPLAY so testcases can connect to Xserver
#	export DISPLAY=:0.0	

        Xvfb :6 -screen scrn 1024x768x16 &
        export DISPLAY=:6 
}

function tc_local_setup()
{
	mXon || return				# X must be running
	tc_exec_or_break  $REQUIRED || return	# need some utilities

	# set approprite image loader
	tc_executes xloadimage && GIF_DISPLAY=xloadimage && return 0
	tc_executes gif2x11 && GIF_DISPLAY=gif2x11 && return 0
	break_if_bad $? "One of xloadimage or gif2x11 is required"
}

function tc_local_cleanup()
{
	[ "$GIF_DISPLAY" ] && {
		tc_info "Several \"terminated\" messages are expected"
		killall $GIF_DISPLAY
		sleep 5 	# let all messages show before continuing
				# with other tests (purely cosmetic!)
	}
        killall -9 Xvfb
        unset DISPLAY
}

################################################################################
# testcase functions
################################################################################
function test1()
{
	tc_register "Installation check"
	tc_executes $PROGS
	tc_pass_or_fail $? "Not all programs in this package are installed" || exit
}

function test2()
{
	tc_register "The solid2 composited with a generated background"
	gifbg -d tl -s 320 200 -c 255 255 255 -l 64 > $TCTMP/bg2.gif
	tc_fail_if_bad $? "unexpected response from gifbg" || return
	gifcomb $GIF_DIR/solid3.gif $TCTMP/bg2.gif > $TCTMP/t2.gif
	tc_fail_if_bad $? "unexpected response from gifcomb" || return
}

function test3()
{
	tc_register "Color density report on the TNHD cover image"
	gifhisto -t $GIF_DIR/solid2.gif >$stdout 2>$stderr
	tc_pass_or_fail $? "unexpected response from gifhisto"
}

function test4()
{
	tc_register "Color histogram of the solid2 image"
	gifhisto -b -s 200 512 $GIF_DIR/solid2.gif \
	   | gifflip -l > $TCTMP/t4.gif
	tc_fail_if_bad $? "unexpected response from gifhisto or gifflip" || return
}

function test5()
{
	tc_register "The #2 solid flipped on its side"
	gifflip -r $GIF_DIR/solid2.gif | gifrsize > $TCTMP/t5.gif 
	tc_fail_if_bad $? "unexpected response from gifflip" || return
}

function test6()
{
	tc_register "solid # 2 flipped on its side"
	gifflip -x $GIF_DIR/solid2.gif > $TCTMP/t6.gif 
	tc_fail_if_bad $? "unexpected response from gifflip" || return
}

function test7()
{
	tc_register "Scale the #2 solid by 0.5"
	gifrsize -s 0.45 $GIF_DIR/solid2.gif > $TCTMP/t7.gif 
	tc_fail_if_bad $? "unexpected response from gifrsize" || return
}

function test8()
{
	tc_register "Reposition the solid # 3 image"
	gifpos -s 720 348 -i 400 148 $GIF_DIR/solid3.gif > $TCTMP/t8.gif
	tc_fail_if_bad $? "unexpected response from gifpos" || return
}

function test9()
{
	tc_register "Resize the #2 solid image"
	gifrsize -S 800 600 $GIF_DIR/solid2.gif > $TCTMP/t9.gif 
	tc_fail_if_bad $? "unexpected response from gifrsize" || return
}

function test10()
{
	tc_register "Clip, crop, and resize the #2 solid image"
	gifclip -i 222 0 390 134 $GIF_DIR/solid2.gif \
	  | gifpos -s 169 135 | gifrsize -s 2.0 > $TCTMP/t10.gif 
	tc_fail_if_bad $? "unexpected response from gifclip or gifpos" || return
}
function test11()
{
	tc_register "Rotate solid # 2 by 45 degrees" 
	gifrotat -a 45 $GIF_DIR/solid2.gif > $TCTMP/t11.gif
	tc_fail_if_bad $? "unexpected response from gifrotat" || return
}

function test12()
{
	tc_register "Copy the solid2 image (test DGifSlurp and DGifSpew)"
	gifspnge <$GIF_DIR/solid2.gif > $TCTMP/t12.gif 
	tc_fail_if_bad $? "unexpected response from gifspnge" || return
}

function test13()
{
	tc_register "Copy a transparent image (test extensions)"
	gifspnge <$GIF_DIR/solid2.gif > $TCTMP/t13.gif 
	tc_fail_if_bad $? "unexpected response from gifspnge" || return
}

################################################################################
# main
################################################################################
TST_TOTAL=13

tc_setup

declare -i RC=0
declare -i i=0
while [ $i -lt $TST_TOTAL ]
do
	let i+=1

	# run test; load image; wait for it to be loaded
	test$i &&
	if [ $i -ne 1 -a $i -ne 3 ] ; then	# do not load image for tests 1 and 3
		$GIF_DISPLAY $TCTMP/t$i.gif >$stdout 2>$stderr &
		declare -i c=0
		declare -i max=10
		while ! ps -ef | grep -v grep | grep -q $GIF_DISPLAY && [ $c -lt $max ]
		do
			sleep 1
			let c+=1
		done
		[ $c -lt $max ]
		tc_pass_or_fail $? "$GIF_DISPLAY did not load image" || continue
	fi
done
