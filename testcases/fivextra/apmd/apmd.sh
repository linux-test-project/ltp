#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		                                      ##
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
# File :	apmd.sh
#
# Description:	Test apmd package
#
# Author:	Andrew Pham, apham@austin.ibm.com
#
# History:	Aug 21 2003 - Created - Andrew Pham
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
#		28 Apr 2004 - (rcp) added installation check
#
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TST_TOTAL=3
REQUIRED="ps grep"

COMMANDS="apm apmd apmsleep on_ac_power"
MANUAL="xapm tailf"
################################################################################

################################################################################
# testcase functions
################################################################################

function installation_check()
{
	tc_register "installation check"

	tc_info "NOTE: commands \"$MANUAL\" to be tested manually"

	tc_executes $COMMANDS $MANUAL
	tc_pass_or_fail $? "apmd not installed" || return
}

function TC_apm()
{	
	tc_register "apm"
	
	apm >$stdout 2>$stderr
	tc_pass_or_fail $? "Unexpected response from apm command." || return
}

function TC_apm-m()
{
	tc_register "apm -m"

	apm -m > $stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from apm -m." || return
	grep -q "min" $stdout
	tc_pass_or_fail $? "Expected to see \"min\" in stdout" || return
}

function TC_apm-v()
{
	register "apm -v"

        apm -v > $stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from apm -v." || return
	grep -q "BIOS" $stdout
	tc_pass_or_fail $? "Expected to see \"BIOS\" in stdout" || return
}

function TC_apmd()
{
	tc_register "apmd"

	local x=`apm`
	declare -i percent=111
	percent="${x:34:2}"

	[ $percent -eq 111 -o apm | grep -q charging ] && { 
		tc_break_if_bad 1 "Unable to get the percent of battery life left"
		return ; }

	local percentW="$(($percent-1))"
        apmd -w $percent -v --wall -p 1 -c 1 > $stdout #2>$stderr
        tc_fail_if_bad $? "Not available." || return

	ps -ef > $stdout 2>/dev/null
	grep -q apmd $stdout >&/dev/null
        tc_fail_if_bad $? "Unable to start apmd." || return

	echo "" > $stdout
	# Wait until 1% of the battery is used up.
	tc_info 'apmd: Wait until 1 percent of the battery is used up.'
	tc_info "apmd: start at $percent percent; end at $percentW percent"
	
	apm | grep -q "$percentW%" >&/dev/null
	RC=$? 
	while [ $RC -ne 0 ]
	do
		apm | grep -q "$percentW%"
		RC=$? 
	done
	
	# Wait for log to sync
	sleep 3s
	
	killall apmd >& /dev/null || tc_info "Unable to kill apmd." 
	grep -q "apmd_call_proxy" /var/log/messages &&
	grep -q "APM BIOS" /var/log/messages
        tc_pass_or_fail $? "Unexpected output." 
}

function TC_apmsleep()
{
	tc_register "apmsleep -dps"
	
	tc_info "***************** WARNING ***************************"
	tc_info "apmsleep:  Your laptop is about to go into suspend mode " 
	tc_info "apmsleep:   for about 1 minute"
	tc_info "apmsleep:  This would turn off everything except the memory."
	tc_info "apmsleep:  The screen will be turned off too; but the whole"
	tc_info "apmsleep:  system should come back in 1 minute."
	tc_info "***************** WARNING ***************************"
	
	sleep 5s
	apmsleep -dsp +00:01 > $stdout 2>$stderr
	tc_pass_or_fail $? "Not installed."
}

function TC_on_ac_power()
{
        tc_register "on_ac_power"

	on_ac_power >$stdout 2>$stderr
	[ $? -eq 1 ]
	tc_fail_if_bad $? "Not available." || return

	grep -q "%" $stdout
	tc_pass_or_fail $? "Unexpected output."
}
################################################################################
# main
################################################################################
TST_TOTAL=7
tc_setup

# Check if supporting utilities are available
tc_exec_or_break $REQUIRED || exit


# Test apmd
installation_check &&
TC_apm &&
TC_apm-m &&
TC_apm-v &&
TC_apmd &&
TC_apmsleep &&
TC_on_ac_power

