#!/bin/bash
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2001                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#                                                                             ##
# File :        at.sh                                                         ##
#                                                                             ##
# Description:  Test basic functionality of at command                        ##
#                                                                             ##
# Author:       Yu-Pao Lee                                                    ##
#                                                                             ##
# History:      Feb 03 2003 - Created - Yu-Pao Lee                            ##
#                                                                             ##
#		16 Dec 2003 - (robert) updated to tc_utils.source
################################################################################


# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# global variables
started_atd="no"

################################################################################
# utility functions for this testcase
################################################################################

# stops atd if atd is atarted in this testcase
# call standard cleanup function
function tc_local_cleanup()
{
	if [ "$started_atd" = "yes" ] ; then
		/etc/init.d/atd stop
		tc_info "Stopped atd."
	fi
}

################################################################################
# testcase functions
################################################################################

#
# test01	atd and at commands
#
function test01()
{
	tc_register "atd and at commands"
    
	tc_exec_or_break grep sleep cat at || return
	
	# ensure atd is up
	if ! /etc/init.d/atd status | grep running &>/dev/null ; then
		tc_root_or_break || return
		/etc/init.d/atd start | grep done >/dev/null
		tc_fail_if_bad $? "atd did not start." || return 
		tc_info "Started atd for this testcase."
		Start_atd="yes"
	fi

	echo "echo hello > $TCTMP/hello.txt" | at now+1 minutes &>$stdout
	tc_fail_if_bad $? "unexpected response from at command"

	# sleep for 61 seconds to wait for the at command to be executed
	tc_info "sleep 61 for at command to be executed."
	sleep 61
 
	# check if the file - hello.txt tc_exist_or_break   
	[ -f $TCTMP/hello.txt ]
	tc_fail_if_bad $? "\"hello.txt\" not found." || return

	# check the content of hello.txt
	cat $TCTMP/hello.txt > $stdout	# for any possible error message
	grep -q "hello" $TCTMP/hello.txt 
	tc_pass_or_fail $? "expected \"hello\" in stdout"
}   

#
# test02	atq and atrm commands
#
function test02()
{
	tc_register "atq and atrm commands"
	
	tc_exec_or_break awk || return

	echo "ls >$TCTMP/ls.txt" | at 1am tomorrow &> $TCTMP/at.queue

	cat $TCTMP/at.queue | grep job &> /dev/null
	tc_fail_if_bad $? "at command failed. - no job number found." \
		|| return 
	
	local job_num=`cat $TCTMP/at.queue | grep job | awk '{print $2}'`
	
	atq | grep -q $job_num >$stdout 2>$stderr
	tc_fail_if_bad $? "atq command failed to list $job_num." || return 

	# delete the job using atrm
	atrm $job_num

	! atq | grep $job_num &>$stdout
	tc_pass_or_fail $? "atrm command failed to delete $job_num."
}

################################################################################
# main function
################################################################################

TST_TOTAL=2

tc_setup		# standard setup

test01 &&
test02
