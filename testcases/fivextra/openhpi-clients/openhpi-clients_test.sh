#!/bin/bash
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
# File :   openhpi-clients_test.sh
#
# Description: This program tests basic functionality of openhpi-clients command.
#
# Author:   Dang En Ren <rende@cn.ibm.com>
#
# History:	Jun 28 2004 - created

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# These are set in tc_local_setup
testcmd_file=""
tests=""

#
# local setup
#
function tc_local_setup()
{
	testcmd_file=$TCTMP/testlist
	cat > $testcmd_file <<-EOF
		hpialarmpanel 
		hpialarmpanel -c1
		hpialarmpanel -c0
		hpialarmpanel -m1
		hpialarmpanel -m0
		hpievent 
		hpievent -x
		hpifan
		hpifru
		hpiinv
		hpireset
		hpisel -c
		hpisel -x
		hpisensor -t
		hpisettime -d 06/28/2004 -t 18:00:00 -x
		hpiwdt -r
		hpiwdt -e
		hpiwdt -d
	EOF
	while read line ; do
		tests="$tests \"$line\""
		(( ++TST_TOTAL ))
	done < $testcmd_file
}

#
# test01	Installation check
#
function test01()
{
	tc_register "openhpi-clients command check"
	eval tc_exec_or_break $tests
}

#
# test02	Test openhpi-clients command
#
function test02()
{
	while read cmd ; do
		tc_register "$cmd"
		$cmd >$stdout 2>$stderr
		tc_pass_or_fail $? "$cmd failed" 
	done < $testcmd_file
}

#
#  test03	Verify hpisettime
#
function test03()
{
	tc_register hpisettime
	tc_exec_or_break date || return

	# get current date and time
	set $(date +"%m %d %Y %H %M %S")
	month=$1; day=$2; year=$3; hour=%4; min=$5; sec=$6

	# set date forward 1 year
	(( ++year ))
	hpisettime -d $month/$day/$year -t $hour:$min$sec >$stdout 2>$stderr
	set $(date +%Y)
	[ "$1" -eq $year ]
	tc_fail_if_bad $? "Expected year to be set to $year but it was $1" || return

	# restore date
	(( --year ))
	hpisettime -d $month/$day/$year -t $hour:$min$sec >$stdout 2>$stderr
	tc_pass_or_fail $? "Unexpected response while restoring date"
}

TST_TOTAL=0	# number of testcases in addition to those listed in
		# $testcmd_file and executed by test02()

tc_setup

tc_root_or_break || exit

test01 &&
test02
#  dummy plugin does not actually set system time, ignored
#test03
