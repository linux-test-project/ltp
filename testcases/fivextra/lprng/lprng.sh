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
# File :	lprng.sh
#
# Description:	Test the lprng print system by printing to a file
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Mar 15 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		16 Dec 2003 - (rcp) updated to tc_utils.source

################################################################################
# source the standard utility functions
################################################################################

cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

restart_lpd="no"
spool_cerated="no"
outfile=""			# filled in by main
lphome=""			# filled in by test01
initscript="/etc/init.d/lpd"
printer="ltp_printer"		# queue name
lpuser=lp			# user name to own print queue
lpgroup=lp			# group name to own print queue


################################################################################
# any utility functions specific to this file can go here
################################################################################

#
# tc_local_cleanup	cleanup specific to this testcase
#
function tc_local_cleanup()
{
	# shut down lpd while reconfiguring
	which $initscript &>/dev/null && $initscript stop &>/dev/null

	# restore /etc/printcap
	[ -s /etc/printcap-$TCID$$ ] && mv /etc/printcap-$TCID$$ /etc/printcap

	# remove $printer spool
	[ "$spool_created" = "yes" ] && rm -rf $lphome/$printer

	# restart lpd if it was originally running
	[ "$restart_lpd" = "yes" ] && $initscript start &>/dev/null

	# remove printed-to file
	rm -rf $outfile &>/dev/null
}

################################################################################
# the testcase functions
################################################################################

#
# test01	install check
#
function test01()
{
	tc_register "install check"
	tc_exec_or_break echo grep cut || return

	local errmsg="lprng not installed"
	local lpinfo="`grep "^$lpuser:" /etc/passwd`"
	[ "$lpinfo" ]
	tc_fail_if_bad $? "$errmsg" "Can't find $lpuser in /etc/passwd" || return

	lphome="`echo $lpinfo | cut -d: -f 6`"
	tc_exist_or_break $lphome
	tc_fail_if_bad $? "$errmsg" "couldn't find $lphome" || return

	[ -x $initscript ]
	tc_pass_or_fail $? "$errmsg" "couldn't find executable $initscript"
}

#
# test02	start lpd
#
function test02()
{
	tc_register "lpd"
	tc_exec_or_break cat touch mv tar || return

	# don't run if spool file already tc_exist_or_break
	[ ! -e $lphome/$printer ]
	tc_break_if_bad $? \
		"Cowardly refusing to clobber existing $lphome/$printer" || return
	spool_created="yes"

	# see if lpd is already running
	if $initscript status | grep OK &>/dev/null ; then
		restart_lpd="yes"
		# shut down lpd while configuring
		$initscript stop > /dev/null
		tc_fail_if_bad $? "Could not do \"$initscript stop\"" || return
	fi

	# save original /etc/printcap, if any
	touch /etc/printcap
	mv /etc/printcap /etc/printcap-$TCID$$

	# create new /etc/printcap
	cat > /etc/printcap <<-EOF
	$printer:\\
		:cm=lpdfilter method=raw color=no:\\
		:lp=|/bin/cat > $outfile:\\
		:sd=$lphome/$printer:\\
		:lf=$lphome/$printer/log:\\
		:af=$lphome/$printer/acct:\\
		:la@:\\
		:tr=:cl:sh:
	EOF

	# create spool directory
	tar zxf $LTPBIN/$printer.tgz -C $lphome
	chown -R $lpuser:$lpgroup $lphome/$printer

	# start lpd
	$initscript start >$stdout 2>$stderr
	tc_pass_or_fail $? "lpd did not start"
}
#
# test03	print to file
#
function test03()
{
	tc_register "lpr"
	tc_exec_or_break grep || return

	# print the message
	local message="successful print"
	echo $message | lpr -P $printer 2>$stderr >$stdout
	tc_fail_if_bad $? "lpr command failed" || return
	tc_info "Waiting 3 sec for print queue to process"
	sleep 3

	# verify that the message was printed to file
	[ -e $outfile ]
	tc_fail_if_bad $? "job did not print" || return

	# verify correct output
	grep "$message" < $outfile >/dev/null
	tc_pass_or_fail $? "message did not print to file" \
		"expected"$'\n'"$message" \
		"got"$'\n'"`cat $outfile`"
}

held_message="held print job"	# for use by tst04 and tst05

#
# test04	lpc holdall
#
function test04()
{
	tc_register "lpc holdall"
	tc_exec_or_break echo || return
	
	rm -f $outfile &>/dev/null

	# hold the queue
	echo holdall | lpc -P $printer >$stdout 2>$stderr
	tc_fail_if_bad $? "bad response" || return

	# print to held queue
	echo $held_message | lpr -P $printer >$stdout 2>$stderr
	tc_fail_if_bad $? "lpr command failed" || return

	# see that job is held
	tc_info "Waiting 3 sec to ensure print queue not processed"
	sleep 3
	lpq -P $printer 2>$stderr | grep -q "^hold"
	tc_fail_if_bad $? "no held job found" || return

	# see that held job did not print
	[ ! -e $outfile ]
	tc_pass_or_fail $? "held job printed but shouldn't have"
}

#
# test05	lpc release
#
function test05()
{
	tc_register "lpc release"

	# release the held print queue
	echo release | lpc -P $printer >$stdout 2>$stderr
	tc_fail_if_bad $? "bad response" || return
	tc_info "Waiting 3 sec for released queue to process"
	sleep 3

	# look for expected output file
	[ -e $outfile ]
	tc_fail_if_bad $? "job did not print when released" || return

	# ensure proper output
	grep -q "$held_message" < $outfile
	tc_pass_or_fail $? "held_message did not print to file" \
		"expected"$'\n'"$held_message" \
		"got"$'\n'"`cat $outfile`"
}

################################################################################
# main
################################################################################

TST_TOTAL=5

tc_setup

tc_root_or_break || exit

outfile="/tmp/$TCID$$"		# not in $TCTMP due to permission issues

test01 && \
test02 && \
test03 && \
test04 && \
test05 
