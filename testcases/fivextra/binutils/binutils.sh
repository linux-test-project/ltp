#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2003                 ##
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
# File :        binutils.sh
#
# Description:  Test binutils package
#
# Author:
#
# History:
#
################################################################################

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# testcase functions
################################################################################

AS=as
AR=ar
NM=nm
OBJCOPY=objcopy
OBJDUMP=objdump
STRIP=strip 
READELF=readelf
SIZE=size
STRINGS=strings
RANLIB=ranlib
TDIR=t_binutils
GCC=`< $TDIR/FIV_CC_NAME.txt`

function check_dep(){

   tc_register "Checking dependency."

   [[ "$GCC" ]]
   tc_break_if_bad $? "Check gcc failed." || return 1

   tc_exec_or_break as ar nm objcopy objdump strip readelf size strings ranlib || return 1                                     
   tc_pass_or_fail $?  "Check dependency failed."                             
} 

function test_ar() {

        local source_name="bintest"

        # register test name
        tc_register "Command ar"

        rm -f ${TCTMP}/${source_name}.o ${TCTMP}/${source_name}.a

        $AS ${source_name}.s -o ${TCTMP}/${source_name}.o >$stdout 2>$stderr
        tc_fail_if_bad $? "Creat ${source_name}.o failed." || return 1 

        $AR -r ${TCTMP}/${source_name}.a ${TCTMP}/${source_name}.o >$stdout 2>$stderr
        tc_fail_if_bad $? "Creat ${source_name}.a failed." || return 1   
    
        $NM --print-armap ${TCTMP}/${source_name}.a > $TCTMP/armap 2>$stderr
        tc_fail_if_bad $? "Can't get armap." || return 1

	grep ".*text_symbol in ${source_name}\.o.*" $TCTMP/armap >/dev/null 2>$stderr && \
        grep ".*data_symbol in ${source_name}\.o.*" $TCTMP/armap >/dev/null 2>$stderr && \
        grep ".*common_symbol in ${source_name}\.o.*" $TCTMP/armap >/dev/null 2>$stderr
        
        tc_pass_or_fail $? " Test command ar failed." 
}


function test_nm() {

        local source_name="bintest"             

        # register test name
        tc_register "Command nm" 

        rm -f ${TCTMP}/${source_name}.o 

        $AS ${source_name}.s -o ${TCTMP}/${source_name}.o >$stdout 2>$stderr  
        tc_fail_if_bad $? "Creat ${source_name}.o failed." || return 1                 


        tc_info "Test command nm with no argument ."

        $NM ${TCTMP}/${source_name}.o > $TCTMP/no_arg_map 2>$stderr  
        tc_fail_if_bad $? "Can't get no_arg_map." || return 1

        grep ".*T.*text_symbol.*" $TCTMP/no_arg_map >/dev/null 2>$stderr && \
        grep ".*D.*data_symbol.*" $TCTMP/no_arg_map >/dev/null 2>$stderr && \
        grep ".*C.*common_symbol.*" $TCTMP/no_arg_map >/dev/null 2>$stderr && \
        grep ".*U.*external_symbol.*" $TCTMP/no_arg_map >/dev/null 2>$stderr && \
        grep ".*t.*static_text_symbol.*" $TCTMP/no_arg_map >/dev/null 2>$stderr && \
        grep ".*d.*static_data_symbol.*" $TCTMP/no_arg_map >/dev/null 2>$stderr 
       
        tc_fail_if_bad  $? "Test command nm with no argument failed." || return 1



        tc_info "Test command nm with -g argument ."

        $NM -g ${TCTMP}/${source_name}.o > $TCTMP/g_map 2>$stderr
        tc_fail_if_bad $? "Can't get g_map." || return 1

        grep ".*text_symbol.*" $TCTMP/g_map >/dev/null 2>$stderr && \
        grep ".*data_symbol.*" $TCTMP/g_map >/dev/null 2>$stderr && \
        grep ".*common_symbol.*" $TCTMP/g_map >/dev/null 2>$stderr && \
        grep ".*external_symbol.*" $TCTMP/g_map >/dev/null 2>$stderr 
                                                                
        tc_pass_or_fail $? " Test command nm with -g argument failed."  


}



function test_objcopy() {                                            
                                                                
        local source_name="bintest"                             
        local copyfile="copy"
                                                                
        # register test name                                    
        tc_register "Command objcopy"                                
                                                                
        rm -f ${TCTMP}/${source_name}.o ${TCTMP}/${copyfile}.o
                                                                
        $AS ${source_name}.s -o ${TCTMP}/${source_name}.o >$stdout 2>$stderr                                                
        tc_fail_if_bad $? "Creat ${source_name}.o failed." || return 1                                                    
                                                                

        tc_info "Test ${OBJCOPY} with no argument ."

        $OBJCOPY ${TCTMP}/${source_name}.o ${TCTMP}/${copyfile}.o >$stdout 2>$stderr 
        tc_fail_if_bad $? "Copy ${source_name}.o to {copyfile}.o failed ." || return 1


        tc_info "Test ${OBJCOPY} with -O spec argument ." 

        local srecfile="${copyfile}.srec"

        $OBJCOPY -O srec ${TCTMP}/${source_name}.o ${TCTMP}/${srecfile} >$stdout 2>$stderr
        #tc_fail_if_bad $? "Test command objcopy with \"-O srec\" argument failed ." || return 1
        tc_fail_if_bad $? "Creat ${srecfile} failed ." || return 1

        grep  '^S[789][0-9a-fA-F]*'  $TCTMP/${srecfile}  >/dev/null  2>$stderr  && \
        grep '^S[123][0-9a-fA-F]*' $TCTMP/${srecfile} >/dev/null 2>$stderr                                    
        tc_fail_if_bad $? "${srecfile} is not correct." || return 1

        $OBJDUMP -f $TCTMP/${srecfile} | grep "file format srec" >/dev/null 2>$stderr
        tc_pass_or_fail $? "Test command ${OBJCOPY} failed."  
         
}                                                       



function test_strip() {
        
        local source_name="testprog"
	if ! tc_executes $GCC ; then
		(( --TST_TOTAL ))
		tc_info "test_strip skipped (no $GCC)"
		return
	fi
        
        # register test name
        tc_register "Command strip"
        
        rm -f ${TCTMP}/${source_name}.o ${TCTMP}/${source_name}.a $TCTMP/allmap
        
        $GCC -c ${source_name}.c -o ${TCTMP}/${source_name}.o >$stdout 2>$stderr
        tc_fail_if_bad $? "Creat ${source_name}.o failed." || return 1
        
        tc_info "Test ${STRIP} by strip ${source_name}.a." 

        $AR -r ${TCTMP}/${source_name}.a ${TCTMP}/${source_name}.o >$stdout 2>$stderr
        tc_fail_if_bad $? "Creat ${source_name}.a failed." || return 1
        
        $STRIP ${TCTMP}/${source_name}.a >$stdout 2>$stderr
        tc_fail_if_bad $? "${STRIP} ${source_name}.a failed." || return 1
        
         
        tc_info "Test ${STRIP} by strip ${source_name}.o."

        $STRIP ${TCTMP}/${source_name}.o >$stdout 2>$stderr                    
        tc_fail_if_bad $? "${STRIP} ${source_name}.o failed." || return 1 

        $NM -a ${TCTMP}/${source_name}.o >$TCTMP/allmap 2>&1
        tc_fail_if_bad $? "Can't get allmap." || return 1
        
        grep ".*: no symbols.*" $TCTMP/allmap >/dev/null 2>$stderr 
        tc_pass_or_fail $? " Test command ${STRIP} failed."

} 


function test_objdump() {
        
        local source_name="bintest"
        
        # register test name
        tc_register "Command objdump"
        
        rm -f ${TCTMP}/${source_name}.o $TCTMP/${OBJDUMP}_i $TCTMP/${OBJDUMP}_f $TCTMP/${OBJDUMP}_t $TCTMP/${OBJDUMP}_r
        
        $AS ${source_name}.s -o ${TCTMP}/${source_name}.o >$stdout 2>$stderr
        tc_fail_if_bad $? "Creat ${source_name}.o failed." || return 1
        

        tc_info "Test ${OBJDUMP} with -i argument."

        $OBJDUMP -i >$TCTMP/${OBJDUMP}_i 2>$stderr
        tc_fail_if_bad $? "Get ${OBJDUMP}_i failed." || return 1       

        grep "BFD header file version.*" $TCTMP/${OBJDUMP}_i >/dev/null 2>$stderr && \
        grep ".*header .*endian.*, data.*endian.*" $TCTMP/${OBJDUMP}_i >/dev/null 2>$stderr 
        tc_fail_if_bad $? "Test ${OBJDUMP} with -i failed." || return 1


        tc_info "Test ${OBJDUMP} with -f argument."

        $OBJDUMP -f ${TCTMP}/${source_name}.o >$TCTMP/${OBJDUMP}_f 2>$stderr
        tc_fail_if_bad $? "Get ${OBJDUMP}_f failed." || return 1
        
        grep "${source_name}.o.*file format" $TCTMP/${OBJDUMP}_f >/dev/null 2>$stderr && \
        grep "architecture:.*" $TCTMP/${OBJDUMP}_f >/dev/null 2>$stderr && \
        grep "HAS_RELOC.*HAS_SYMS" $TCTMP/${OBJDUMP}_f >/dev/null 2>$stderr 
        tc_fail_if_bad $? "Test ${OBJDUMP} with -f failed." || return 1


        tc_info "Test ${OBJDUMP} with -t argument."
        
        $OBJDUMP -t ${TCTMP}/${source_name}.o >$TCTMP/${OBJDUMP}_t 2>$stderr
        tc_fail_if_bad $? "Get ${OBJDUMP}_t failed." || return 1
        
        grep ".*text_symbol" $TCTMP/${OBJDUMP}_t >/dev/null 2>$stderr && \
        grep ".*data_symbol" $TCTMP/${OBJDUMP}_t >/dev/null 2>$stderr && \
        grep ".*external_symbol" $TCTMP/${OBJDUMP}_t >/dev/null 2>$stderr && \
        grep ".*common_symbol" $TCTMP/${OBJDUMP}_t >/dev/null 2>$stderr 
        tc_fail_if_bad $? "Test ${OBJDUMP} with -t failed." || return 1


        tc_info "Test ${OBJDUMP} with -r argument."
        
        $OBJDUMP -r ${TCTMP}/${source_name}.o >$TCTMP/${OBJDUMP}_r 2>$stderr
        tc_fail_if_bad $? "Get ${OBJDUMP}_r failed." || return 1
       
        grep "${source_name}.o.*file format" $TCTMP/${OBJDUMP}_r >/dev/null 2>$stderr && \
        grep "RELOCATION RECORDS FOR" $TCTMP/${OBJDUMP}_r >/dev/null 2>$stderr && \
        grep "external_symbol" $TCTMP/${OBJDUMP}_r >/dev/null 2>$stderr
        tc_pass_or_fail $? "Test ${OBJDUMP} with -r failed." 


}       





function test_readelf() {
                                                                
        local source_name="testprog"                             
	if ! tc_executes $GCC ; then
		(( --TST_TOTAL ))
		tc_info "test_readlf skipped (no $GCC)"
		return
	fi
                                                                
        # register test name                                    
        tc_register "Command readelf"                           
                                                                
        rm -f ${TCTMP}/${source_name}  $TCTMP/${READELF}_h  $TCTMP/${READELF}_l                       
                                                                
        $GCC ${source_name}.c -o ${TCTMP}/${source_name} >$stdout 2>$stderr                                               
        tc_fail_if_bad $? "Creat ${source_name} failed." || return 1                                                     
                                                                
                                                                
        tc_info "Test ${READELF} with -h argument."             
                                                                
        $READELF -h ${TCTMP}/${source_name} >$TCTMP/${READELF}_h 2>$stderr              
        tc_fail_if_bad $? "Get ${READELF}_h failed." || return 1
                                                                
        grep "ELF Header:" $TCTMP/${READELF}_h >/dev/null 2>$stderr && \
        grep "Class:" $TCTMP/${READELF}_h >/dev/null 2>$stderr && \
        grep "Type:" $TCTMP/${READELF}_h >/dev/null 2>$stderr                                
        tc_fail_if_bad $? "Test ${READELF} with -h failed." || return 1                                                    
                                                                
                                                                
        tc_info "Test ${READELF} with -l argument."
        
        $READELF -l ${TCTMP}/${source_name} >$TCTMP/${READELF}_l 2>$stderr
        tc_fail_if_bad $? "Get ${READELF}_l failed." || return 1
        
        grep "Elf file type is" $TCTMP/${READELF}_l >/dev/null 2>$stderr && \
        grep "Program Headers:" $TCTMP/${READELF}_l >/dev/null 2>$stderr 
        tc_pass_or_fail $? "Test ${READELF} with -l argument failed."


}


function test_size() {

        local source_name="bintest"

        # register test name
        tc_register "Command size"

        rm -f ${TCTMP}/${source_name}.o  $TCTMP/${SIZE}  $TCTMP/${SIZE}_A 

        $AS ${source_name}.s -o ${TCTMP}/${source_name}.o >$stdout 2>$stderr
        tc_fail_if_bad $? "Creat ${source_name}.o failed." || return 1


        tc_info "Test ${SIZE} with no argument."

        $SIZE ${TCTMP}/${source_name}.o >$TCTMP/${SIZE} 2>$stderr
        tc_fail_if_bad $? "Can't get ${SIZE}." || return 1

        grep "[0-9]*.*[0-9]*.*[0-9]*.*[0-9]*.*[0-9a-fA-F]*.*${source_name}.o" $TCTMP/${SIZE} >/dev/null 2>$stderr
        tc_fail_if_bad $? " Test ${SIZE} with no argument failed." || return 1


        tc_info "Test ${SIZE} with -A argument."

        $SIZE -A ${TCTMP}/${source_name}.o >$TCTMP/${SIZE}_A 2>$stderr
        tc_fail_if_bad $? "Can't get ${SIZE}_A." || return 1
        
        grep "${source_name}.o" $TCTMP/${SIZE}_A >/dev/null 2>$stderr && \
        grep -E 'text.*[0-9]*.*[0-9]*|TEXT.*[0-9]*.*[0-9]*' $TCTMP/${SIZE}_A >/dev/null 2>$stderr && \
        grep -E 'data.*[0-9]*.*[0-9]*|DATA.*[0-9]*.*[0-9]*' $TCTMP/${SIZE}_A >/dev/null 2>$stderr
        tc_pass_or_fail $? " Test ${SIZE} with -A argument failed."
        
} 

function test_strings () {

        local source_name="bintest"

        # register test name
        tc_register "Command strings"

        rm -f ${TCTMP}/${source_name}.o $TCTMP/${STRINGS}_a 

        $AS ${source_name}.s -o ${TCTMP}/${source_name}.o >$stdout 2>$stderr
        tc_fail_if_bad $? "Creat ${source_name}.o failed." || return 1
        
        
        tc_info "Test ${STRINGS} with -a argument."
        
        $STRINGS -a ${TCTMP}/${source_name}.o >$TCTMP/${STRINGS}_a 2>$stderr
        tc_fail_if_bad $? "Can't get ${STRINGS}_a." || return 1
        
        grep "text_symbol" $TCTMP/${STRINGS}_a >/dev/null 2>$stderr && \
        grep "external_symbol" $TCTMP/${STRINGS}_a >/dev/null 2>$stderr && \
        grep "data_symbol" $TCTMP/${STRINGS}_a >/dev/null 2>$stderr && \
        grep "common_symbol" $TCTMP/${STRINGS}_a >/dev/null 2>$stderr 
        tc_pass_or_fail $? " Test ${STRINGS} with -a argument failed."
        
}       



function test_ranlib () {
        
        local source_name="bintest"
        
        # register test name
        tc_register "Command ranlib"
        
        rm -f ${TCTMP}/${source_name}.o 
        
        tc_info "Test ${RANLIB} ."

        $AS ${source_name}.s -o ${TCTMP}/${source_name}.o >$stdout 2>$stderr
        tc_fail_if_bad $? "Creat ${source_name}.o failed." || return 1

        $AR -Sr ${TCTMP}/${source_name}.a ${TCTMP}/${source_name}.o >$stdout 2>$stderr
        tc_fail_if_bad $? "Creat ${source_name}.a failed." || return 1

        local ret=`nm -s ${TCTMP}/${source_name}.a|grep -i "Archive.*index.*:"|wc -l`
        tc_fail_if_bad $ret "${source_name}.a is not correct." || return 1
        
        $RANLIB ${TCTMP}/${source_name}.a >$stdout 2>$stderr
        tc_fail_if_bad $ret "Generate archive symbol table failed." || return 1

        nm -s ${TCTMP}/${source_name}.a | grep -i "Archive.*index.*:" >/dev/null 2>$stderr 
        tc_pass_or_fail $? " Test ${RANLIB} failed."
        
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

TST_TOTAL=10
tc_setup
cd $TDIR	# testcase data is here

check_dep || exit 1
test_ar
test_nm         
test_objcopy
test_strip
test_objdump
test_readelf
test_size
test_strings
test_ranlib
