#!/bin/sh
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2004						      ##
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
# File :	tk_test.sh
#
# Description:	Test the tk package.
#
# Author:	csdl
#
# History:	Jun 28 2004 - Created
#		Jun 28 2004 - RCP: added Additional error output.
#		Aug 03 2004 - Modified by Gong Jie gongjie@cn.ibm.com
#			    - Startup and shutdown Xvfb automatically.

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TCLCMD=""
TESTDIR="tktests"
REQUIRED="awk grep Xvfb"

################################################################################
# utility functions
################################################################################

tc_local_setup()
{	
	TCLCMD=`which tclsh`
	[ "$TCLCMD" ]
	tc_break_if_bad $? "no tclsh installed" || return

	tc_exec_or_break $REQUIRED || return
	
	cd $TESTDIR
	
#	TCLCMD  $PWD/prep.test > $TCTMP/prep.txt 2>&1
#	grep "can't find package tk" $TCTMP/prep.txt
#	if [ $? -eq 0 ] ; then
#	        tc_info "No tk instlled."
#	        exit 1
#	fi
	Xvfb :6 -screen scrn 1024x768x16 &
	export DISPLAY=:6
}

tc_local_cleanup()
{
	killall -9 Xvfb
	unset DISPLAY
}

################################################################################
#  test functions
################################################################################

haveError=0
haveExecuted=0
ErrorInfo=""
ExecSum=""

handle_test_result() 
{ 
	local aline=""
		
	haveError=1
	haveExecuted=0
	ErrorInfo=""
	ExecSum="There is some error in the test!"
	
#	cat $TCTMP/testresult.txt
	while read aline
	do
		ErrorInfo=`echo $aline | grep "FAILED" | awk '{ if ($4 != "")  print $0}' | cut -d " " -f 2-`
		if [ "$ErrorInfo" != "" ] ; then
			haveError=2
			tc_info "$ErrorInfo"
		fi

		ErrorInfo=`echo $aline | grep Total | grep Passed | grep Skipped | grep Failed`
		if [ "$ErrorInfo" != "" ] ; then 
			haveExecuted=1
			if [ $haveError -eq 1 ] ; then
				haveError=0
			fi
			ExecSum=`echo $aline | cut -d " " -f 2-`
		fi
	done < $TCTMP/testresult.txt
}

test_template()
{
	tc_register	"$1"
	echo "cmd=$TCLCMD $PWD/$1"
	$TCLCMD $PWD/$1 > $TCTMP/testresult.txt 2>&1	
	handle_test_result 
	tc_pass_or_fail $haveError "$ExecSum" "$(< $TCTMP/testresult.txt)"
}

################################################################################
#  main
################################################################################
TST_TOTAL=62
tc_setup

test_template bell.test
test_template bgerror.test
test_template bitmap.test
test_template border.test
test_template button.test
test_template canvImg.test
test_template canvPs.test
test_template canvRect.test
test_template canvWind.test
test_template choosedir.test
test_template clipboard.test 	
test_template clrpick.test
test_template cmds.test
test_template color.test
test_template config.test
test_template cursor.test
test_template dialog.test
test_template embed.test
test_template filebox.test
test_template geometry.test
test_template get.test
test_template grab.test
test_template id.test
test_template imgBmap.test
test_template imgPPM.test
test_template imgPhoto.test
test_template macEmbed.test
test_template macFont.test
test_template macMenu.test
test_template macWinMenu.test
test_template macscrollbar.test
test_template main.test
test_template menu.test
test_template menuDraw.test
test_template menubut.test
test_template message.test
test_template msgbox.test
test_template obj.test
test_template oldpack.test
test_template option.test
test_template panedwindow.test
test_template safe.test
test_template scrollbar.test
test_template textBTree.test
test_template textImage.test
test_template textIndex.test
test_template textMark.test
test_template tk.test
test_template unixButton.test
test_template unixMenu.test
test_template util.test
test_template visual.test
test_template visual_bb.test
test_template winButton.test
test_template winClipboard.test
test_template winDialog.test
test_template winFont.test
test_template winMenu.test
test_template winSend.test
test_template winWm.test
test_template window.test
test_template xmfbox.test

# testcases that may have problems
			
## Tk_BindEvent, MatchPatterns failed on XFree86 and fluxbox
##test_template bind.test
#test_template canvText.test		

## CanvasSetOrgin failed on XFree86 and fluxbox
##test_template canvas.test

## EntryWidgetCmd, ConfigureEntry, EntryComputeGeometry, InsertChars, EntrySetValue failed on XFree86 and fluxbox
##test_template entry.test 		
#test_template event.test			

## Tk_FocusCmd failed on fluxbox window manager
##test_template focus.test
#test_template focusTcl.test
#test_template font.test		
#test_template frame.test			

## Failed on XFree86 and fluxbox
##test_template grid.test	
#test_template image.test			
#test_template listbox.test 		
#test_template pack.test

## Faild in all condition
##test_template place.test
#test_template raise.test			

## Failed on XFree86 and fluxbox
##test_template scale.test
#test_template select.test			
#test_template send.test

## Failed on XFree86 and fluxbox
##test_template spinbox.test			
#test_template text.test	

## Failed on fluxbox window manager
##test_template textDisp.test			
#test_template textTag.test
#test_template textWind.test			

## Failed on Xfbdev and mwm
##test_template unixEmbed.test	
#test_template unixFont.test			
#test_template unixSelect.test		

## Faild in all condition
##test_template unixWm.test

## Only passed on mwm window manager
##test_template winfo.test
## Only passed on Xfbdev and mwm window manager
#test_template wm.test
