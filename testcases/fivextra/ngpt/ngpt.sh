#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003		      				      ##
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
# File :	ngpt.sh
#
# Description:	test ngpt basic apis and thread contention for a monitor
#		(mutex and cond var)
#
# Author:	Helen Pang. hpang@us.ibm.com
#
# History:	June 24 2003 - Add Bob Paulsen's sync.c into ngpt testcase
#		June 23 2003 - Created. Helen Pang. hpang@us.ibm.com
#		19 Feb 2003 (rcp) BUG 6432: added ngptinit.
#		06 May 2004 (rcp) Added psync_orig; general cleanup
#

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source
#		16 Dec 2003 - (hpang) updated to tc_utils.source

################################################################################
# the testcase functions
################################################################################

#
# test01
#
function test01()
{
	tc_register	"installation check and initialization"
	tc_executes ngptinit
	tc_fail_if_bad $? "ngpt not installed properly" || return

	ngptinit &>$stdout	# good message goes to stderr
	tc_pass_or_fail $? "could not initialize ngpt threading"
}

#
# do_test	run a named test
#		$1 = test number
#
function do_test()
{
	tc_register "${tc_name[$1]}"
	${tc_cmd[$1]} >$stdout 2>$stderr
	tc_pass_or_fail $? "unexpected results"
}

################################################################################
# main
################################################################################

#
# the tests
#
tc_cmd[1]=pcreate	; tc_name[1]="pthread_create"
tc_cmd[2]=pcancel	; tc_name[2]="pthread_cancel"
tc_cmd[3]=pjoin		; tc_name[3]="pthread_join"
tc_cmd[4]=psync_glibc	; tc_name[4]="pthread_mutex and pthread_cond (glibc)"
tc_cmd[5]=psync		; tc_name[5]="pthread_mutex and pthread_cond (ngpt)"
tc_count=5

let TST_TOTAL=tc_count+1

tc_setup		# standard setup

test01 || exit		# installation check

# run the functional tests
tc_nbr=1
while [ $tc_nbr -le $tc_count ] ; do
	do_test $tc_nbr || break
	let tc_nbr+=1
done
