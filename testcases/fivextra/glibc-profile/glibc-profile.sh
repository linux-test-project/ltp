#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2004						      ##
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
# File :        glibc-profile.sh
#       
# Description:  Test glibc-profile package
#       
# Author:	CSDL
#       
# History:	Jun 28 2004 - created
#       
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# testcase functions
################################################################################


TESTDIR="glibc-profile_test"

# tests is the list of our test suites                          
tests="anl c crypt db1 db2 dl m nsl pthread resolv rt util" 

function check_dep(){
    
   tc_register "Checking dependency."
    
   tc_exec_or_break gcc gprof || return
  
 }


function runtests() {                                           
                                                                
local full_path="${PWD}/${TESTDIR}"                                                                

# run each testcase                                             
for tst in $tests ; do                                          
                  
        local source_name="tst_lib${tst}.c"
        local target_name="tst_lib${tst}"
                                              
        # register test name 
        tc_register "$target_name"                                      
                                                                
        cd $full_path

        rm -f gmon.out $target_name phones.db phones2.db test_rt

        # make sure test file exists                            
        [ -f $source_name ]                                    
        tc_break_if_bad "$?" "The test ${full_path}/${source_name} does not exist." || continue                                         
        # creat test program
        gcc -pg $source_name -o $target_name -l${tst} 1>/dev/null 2>&1 
        tc_fail_if_bad "$?" "Creat ${target_name} failed." || continue
  
        # run it  
        ./$target_name 1>/dev/null 2>&1
        tc_fail_if_bad "$?" "Run ${target_name} failed."  || continue

        # make sure gmon.out exists     
        [ -f gmon.out ]
        tc_break_if_bad "$?" "The ${target_name} didn't creat gmon.out." || continue
       
        # use gprof to test all 
        gprof $target_name 1>/dev/null 2>&1
        tc_pass_or_fail "$?" "$target_name output unexpected."
                                                                
        rm -f gmon.out $target_name phones.db phones2.db test_rt 

done                                                            
}                                                               

####################################################################################                                        
# MAIN
####################################################################################

# Function:     main
#
# Description:  - Execute all tests, report results
#
# Exit:         - zero on success
#               - non-zero on failure
#

set $tests
TST_TOTAL=$#

tc_setup

check_dep || exit 1
runtests

