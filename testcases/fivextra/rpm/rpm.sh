#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab:
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2003						      ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MEECHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :       rpm.sh
#
# Description:  This testcase tests 4 commands in the rpm package.
#		rpm gendiff rpm2cpio rpmqpack
#              
# 
# Author:       Andrew Pham, apham@us.ibm.com
#
# History:      Feb 03 2003 - Created - Andrew Pham.
#		Mar 12 2003 - changed the testcases to use tc_utils.source
#		 	and changed test condition to better aproach.
#   			Andrew Pham
#		Oct 08 2003 - Change test to use rpm created in test01.
#			Andrew Pham
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
#		04 Aug 2004 (rcp) can't test gendiff if find command is busybox
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# Total number of testcases in this file.
TST_TOTAL=7

summary2="Failed: Not available."
summary3="Failed: Unexpected output".
REQUIRED="rpm gendiff rpmqpack rpm2cpio cat mkdir grep"
################################################################################
# makesure to un-install the test rpm in case exit before rpm -e
function tc_local_cleanup()
(
	rpm -e tst_rpm >&/dev/null
)
################################################################################
# the testcase functions
################################################################################

#
# test01    rpm -bs	
#
function test1()
{
	tc_register "rpm -ba"	

	# Create a spec file tst_file.spec for rpm -b
	cat > /usr/src/packages/SPECS/tst_rpm.spec <<-EOF 
	Summary: Dummy package used to test file command
	Name: tst_rpm  
	Version: 1.0
	Release: 1
	Copyright: GPL
	Group: LillB test case
	Source: test1.c

	%description
	A test RPM package used for testing rpm command.

	%prep

	%build

	%install
	mkdir -p /tmp/for_my_test
	cp ../SOURCES/test1.c /tmp/for_my_test
	cp ../SPECS/tst_rpm.spec /tmp/for_my_test


	%files 
	%defattr(-,root,root)
	/tmp/for_my_test/*
	EOF

	[ -d /usr/src/packages/SOURCES ] || mkdir -p /usr/src/packages/SOURCES
				
	cat > /usr/src/packages/SOURCES/test1.c <<-EOF
		#include <stdio.h>

		int main()
		{
		    printf("Hello\n");
		    return 0;
		}
	EOF

	# Actual test begins
	rpm -ba /usr/src/packages/SPECS/tst_rpm.spec > $stdout 2>/dev/null
	tc_fail_if_bad $? "$summary2" || return 

	NEW_RPM=`grep Wrote $stdout | grep -v src.rpm`

	[ "$NEW_RPM" ]
	tc_pass_or_fail $? "$summary3"
}

#
# test02    rpm -i
#
function test2()
{
	tc_register "rpm -i"	

	tc_info "The spec file to build rpm_tst-1.0-1.*.rpm is"
	tc_info "included in rpm_tst-1.0-1.*.rpm itself."

	# install the rpm just built
	set $NEW_RPM
	THE_RPM=$2

	rpm -i $THE_RPM >$stdout 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	[ -e /tmp/for_my_test/tst_rpm.spec -a /tmp/for_my_test/test1.c ]
	tc_pass_or_fail $?  "$summary3"
}
#
# test03    rpm -q
#
function test3()
{
	tc_register "rpm -q"	

	rpm -q tst_rpm >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return 1

	rpm -ql tst_rpm | grep test1.c &>/dev/null
	tc_pass_or_fail $?  "$summary3"
}

#
# test06    gendiff
#
function test6()
{
	# can't test gendiff if find command is busybox version
	tc_is_busybox find && (( --TST_TOTAL )) && return 0
	tc_register "gendiff"
	
	mkdir $TCTMP/diff &>/dev/null

	cat > $TCTMP/diff/t <<-EOF
		one 1
		two 2
		three 3
	EOF
	cat > $TCTMP/diff/tt <<-EOF
		four 4
		five 5
		six 6
	EOF
	cat > $TCTMP/diff/t.orig <<-EOF
		one
		two
		three
	EOF
	cat > $TCTMP/diff/tt.orig <<-EOF
		four
		five
		six
	EOF
	cd $TCTMP
	gendiff diff .orig >$stdout 2>$stderr
	[ $? -ne 0 ]
	tc_fail_if_bad $?  "$summary2" || return

	gendiff diff .orig | grep '+five 5'  &>/dev/null
	tc_pass_or_fail $?  "$summary3"
}
#
# test07   rpm2cpio
#
function test7()
{
	tc_register "rpm2cpio"
	
	rpm2cpio $THE_RPM >$stdout 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return
	
	grep -q Hello $stdout
	tc_pass_or_fail $?  "$summary3"
}
#
# test04    rpmqpack
#
function test4()
{
	tc_register "rpmqpack"
	
	rpmqpack >$stdout 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return 
	
	rpmqpack | grep tst_rpm &>/dev/null
	tc_pass_or_fail $?  "$summary3"
}

#
# test05    rpm -e
#
function test5()
{
	tc_register "rpm -e"	
		
	rpm -e tst_rpm >$stdout 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return 

	[ ! -e /tmp/for_my_test/tst_rpm.spec ]
	tc_pass_or_fail $?  "$summary3"
}
################################################################################
# main
################################################################################
tc_setup

tc_root_or_break || exit
tc_exec_or_break $REQUIRED || exit

FRC=0
i=1
while [ $i -lt 8 ]
do
	test$i || FRC=$? 
	let i+=1
done
exit $FRC
