#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab:
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
# File:		susefirewall2_test.sh
#
# Description:	This program tests basic functionality of ltrace program
#
# Author:	CSDL,  James He <hejianj@cn.ibm.com>
#
# History:	25 May 2004 - created - James He

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# Utility functions 
################################################################################

#
# local setup
#	 
function tc_local_setup()
{	
        tc_root_or_break || return
	tc_exec_or_break ifconfig iptables iptables-save iptables-restore || return

	# incompatible with ipchains
	rmmod ipchains &>/dev/null

	# force load of iptables module
	iptables -L &>/dev/null

	# save current rules, if any
	iptables-save -c >$stdout 2>$stderr
	tc_fail_if_bad $? "iptables-save failed to save the current rule-set" \
		|| return
	cp $stdout $TCTMP/rule.old
}

#
# local cleanup
#	 
function tc_local_cleanup()
{	
	[ -s $TCTMP/rule.old ] && iptables-restore -c <  $TCTMP/rule.old
	# retore rules saved at beginning
	[ -s $TCTMP/rule.old ] && {
		iptables-restore -c < $TCTMP/rule.old >$stdout 2>$stderr
		tc_fail_if_bad $? "iptables_restore failed to restore rule-set" \
			|| return
	}
}

function get_active_interfaces()
{      
	while read line; do
		set -- $line
		case "$3" in
			*UP*)
			dev=${2%%:}
			[ "$dev" != "lo" ] && echo $dev
			;;
		esac
	done < <(ip -o link show)
}
################################################################################
# Testcase functions
################################################################################

#
# test01	installation check
#
function test01()
{
	tc_register	"installation check"
	tc_executes /sbin/SuSEfirewall2
	tc_pass_or_fail $? "SuSEfirewall2 not installed properly"

}

function test02()
{
	tc_register	"configure check"
	firstdev=`get_active_interfaces`
	while read line; do
	        echo $line | grep -e "^FW_DEV_EXT*" >/dev/null
	        if [ "$?" -eq "0" ] ; then
	                echo "FW_DEV_EXT=\"$firstdev\"" >>$TCTMP/ok.conf
	        else
	                echo $line >>$TCTMP/ok.conf
	        fi
	done < susefirewall2.conf
	SuSEfirewall2 file $TCTMP/ok.conf >$stdout 2>&1
	cat $stdout | grep -v "Warning" >/dev/null
	tc_pass_or_fail $? "SuSEfirewall not configured properly"

}

function test03()
{
	tc_register	"SuSEfirewall2 strategy test"
	iptables -L | grep http >>$TCTMP/strategyoutput
	cat $TCTMP/strategyoutput | grep -q "^ACCEPT$tcp$anywhere$anywhere$http*" 
	tc_pass_or_fail $? "SuSEfirewall2 not worked properly"
}

function test04()
{
	tc_register	"stop SuSEfirewall"
	SuSEfirewall2 stop >$stdout 2>&1
	cat $stdout | grep -q "clearing rules now ... done"
	tc_pass_or_fail $? "SuSEfirewall not stopped properly"

}
################################################################################
# main
################################################################################

TST_TOTAL=4

tc_setup

test01 || exit
test02 &&
test03 &&
test04
