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
# File :   sg3_utils.sh
#
# Description: This program tests  functionality of sg3_utils commands.
#
# Author:   Xie Jue <xiejue@cn.ibm.com>
#
# History:	May 17 2004 - created - Xie Jue
#		Jul 8 2004 - ignore some SGIO mmap test on ppc64

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

commands="sg_scan sg_map sg_inq sginfo sg_readcap sg_start sg_modes sg_logs sg_senddiag sg_read sg_dd sg_rbuf sg_test_rwbuf sg_turs sg_reset"
commands_ext="sgm_dd sgp_dd"


# report unimplemented test
# these are tests that should be implemented eventually
#
#       $1      any additional text to print
#
function unimplemented()
{
        tc_info "$TCNAME:        not yet tested. $1"
	}

#
# report a test must be done manually.
# procedure for how to do it is in each test heading.
#
function test_manually()
{
	tc_info "$TCNAME must be tested manually."
	}

#
# TC_sg_scan rather simple but useful program scans the sg devices 
#
function TC_sg_scan()
{
	tc_register "sg_scan test"
	sg_scan >$stdout 2>$stderr
	grep -q "scsi" $stdout  &&
	grep -q "channel=" $stdout &&
	grep -q "lun=" $stdout &&
	grep -q "type=" $stdout	
	tc_fail_if_bad $? "sg_scan fail" || return	
	sg_scan -i >$stdout 2>$stderr
	tc_fail_if_bad $? "sg_scan -i" || return
	sg_scan -n >$stdout 2>$stderr
	tc_fail_if_bad $? "sg_scan -n" || return
	sg_scan -x >$stdout 2>$stderr
	tc_pass_or_fail $? "sg_scan -x" 
}
#
# TC_sg_map shows the mapping of the avialable sg device name 
# sg_map  [-n] [-x] [-sd] [-scd or -sr] [-st]'
#
function TC_sg_map()
{
	tc_register "sg_map test"
	sg_map >$stdout 2>$stderr
	grep -q $device_sg $stdout  
	tc_fail_if_bad $? "sg_map "  || return
	sg_map -n >$stdout 2>$stderr
	tc_fail_if_bad $? "sg_map -n" || return
	sg_map -x >$stdout 2>$stderr
	tc_fail_if_bad $? "sg_map -x" || return
	sg_map -sd >$stdout 2>$stderr
	tc_fail_if_bad $? "sg_map -sd" || return
	local   device_sd_old=`sg_map -sd|grep sd 2>/dev/null|line`
	device_sd=${device_sd_old##* }
# If there is scsi disk mapped as SG device, we use the $device_sg
	if [ -n "$device_sd" ] ; then
		device_sg=`sg_map|grep "$device_sd" | line| cut -d " " -f 1`
	fi
	sg_map -scd >$stdout 2>$stderr
	tc_fail_if_bad $? "sg_map -scd" || return
	sg_map -st >$stdout 2>$stderr
	tc_pass_or_fail $? "sg_map -st" 
}

#
# TC_sg_inq sg_inq executes a SCSI INQUIRY command on the given device and interprets the results
#sg_inq [-e] [-h|-r] [-o=<opcode_page>] [-V] <sg_device>
#
function TC_sg_inq()
{
	tc_register    "sg_inq executes a SCSI INQUIRY command on the given device and interprets the results"
	sg_inq -V >$stdout 2>&1
	tc_fail_if_bad $? "sg_inq -V"  || return
	sg_inq -e $device_sg >$stdout 2>$stderr
	tc_fail_if_bad $? "sg_inq -e $device_sg"  || return
	sg_inq -h $device_sg >$stdout 2>$stderr
	tc_fail_if_bad $? "sg_inq -h $device_sg"  || return
	sg_inq -r $device_sg >$stdout 2>$stderr
	tc_fail_if_bad $? "sg_inq -r $device_sg"  || return
	sg_inq -c $device_sg >$stdout 2>$stderr
	tc_fail_if_bad $? "sg_inq -c $device_sg"  || return
	sg_inq -p $device_sg >$stdout 2>$stderr
	tc_fail_if_bad $? "sg_inq -p $device_sg"  || return
	sg_inq -36 $device_sg >$stdout 2>$stderr
	tc_pass_or_fail $? "sg_inq"
}

#
#TC_sginfo
#
function TC_sginfo()
{
	tc_register    "sginfo"
	sginfo -l >$stdout 2>$stderr
	tc_fail_if_bad $? "sginfo -l"  || return
	sginfo -c $device_sg >$stdout 2>$stderr
	tc_fail_if_bad $? "sginfo -c $device_sg"  || return
	sginfo -C $device_sg >$stdout 2>$stderr
	tc_fail_if_bad $? "sginfo -C $device_sg"  || return
	sginfo -a $device_sg >$stdout 2>$stderr
	tc_pass_or_fail $? "sginfo -a $device_sg"  
}
#
#TC_sg_readcap
#
function TC_sg_readcap()
{
	tc_register    "sg_readcap"
	sg_readcap $device_sg >$stdout 2>$stderr
	tc_pass_or_fail $? "sg_readcap $device_sg"  
}
#
#TC_sg_start
#
function TC_sg_start()
{
	test_manually "sg_start"
}
#
#TC_sg_modes
#
function TC_sg_modes()
{
	tc_register    "sg_modes"
	sg_modes -a $device_sg >$stdout 2>$stderr
	tc_pass_or_fail $? "sg_modes -a $device_sg"  
}
#
#TC_sg_logs
#
function TC_sg_logs()
{
	tc_register    "sg_logs"
	sg_logs -a $device_sg >$stdout 2>$stderr
	tc_pass_or_fail $? "sg_logs -a $device_sg"  
}
#
#TC_sg_senddiag
#
function TC_sg_senddiag()
{
	tc_register "sg_senddiag"
	sg_senddiag -e $device_sg >$stdout 2>$stderr
	tc_fail_if_bad $? "sg_senddiag -e $device_sg"  || return
	sg_senddiag -h $device_sg >$stdout 2>&1
	tc_fail_if_bad $? "sg_senddiag -h $device_sg"  || return
	sg_senddiag -l $device_sg >$stdout 2>&1
	tc_pass_or_fail $? "sg_senddiag -l $device_sg"  
}
#
#TC_sg_reset
#
function TC_sg_reset()
{
	tc_register    "sg_reset $device_sg"
	sg_reset -d $device_sg >$stdout 2>$stderr
	tc_fail_if_bad $? "sg_reset -d $device_sg"  || return
	sleep 5
	sg_reset -h $device_sg >$stdout 2>$stderr
	tc_fail_if_bad $? "sg_reset -h $device_sg"  || return
	sleep 5
	sg_reset -b $device_sg >$stdout 2>$stderr
	tc_pass_or_fail $? "sg_reset -b $device_sg"  
	sleep 5
}
#
#TC_sg_read
#
function TC_sg_read()
{
	if [ -z "$device_sd" ] ; then
		tc_info "No SCSI DISK maps as SG deivce,SKIP sg_read test!"
		return
	fi
	tc_register "sg_read"
	sg_read if=$device_sg bs=512 count=45 >$stdout 2>&1
	grep -q "45+0 records in" $stdout 
	tc_pass_or_fail $? "sg_read: $stdout"
}
#
#TC_sg_dd
#
function TC_sg_dd()
{
	if [ -z "$device_sd" ] ; then
		tc_info "No SCSI DISK maps as SG deivce,SKIP sg_dd test!"
		return
	fi
	tc_register "sg_dd"
	sg_dd if=$device_sg of=$TCTMP/sg.dat bs=512 count=45 >$stdout 2>&1
	grep -q "45+0 records out" $stdout 
	tc_pass_or_fail $? "sg_dd: $stdout"
}
#
#TC_sg_rbuf
#
function TC_sg_rbuf()
{
	if [ -z "$device_sg" ] ; then
		tc_info "No SCSI DISK maps as SG deivce,SKIP sg_rbuf test!"
		return
	fi
	tc_register "sg_rbuf"
	sg_rbuf -q -b=10 -s=20  $device_sg >$stdout 2>$stderr	
	tc_fail_if_bad $? "sg_rbuf -q -b=10 $device_sg" || return
	sg_rbuf -q -b=10 -t  $device_sg >$stdout 2>$stderr	
	tc_fail_if_bad $? "sg_rbuf -q -b=10 -t $device_sg" || return
	sg_rbuf -d -b=10 -s=20  $device_sg  >$stdout 2>$stderr	
	tc_fail_if_bad $? "sg_rbuf -d -b=10 -s=20  $device_sg" || return
	[ "$build_host" = "ppc64" ] || {
	sg_rbuf -m -b=10 -s=20  $device_sg  >$stdout 2>$stderr	
	tc_pass_or_fail $? "sg_rbuf -m -b=10 -s=20  $device_sg" 
	}
}
#
#TC_sg_test_rwbuf
#
function TC_sg_test_rwbuf()
{
	if [ -z "$device_sg" ] ; then
		tc_info "No SCSI DISK maps as SG deivce,SKIP sg_test_rwbuf test!"
		return
	fi
	tc_register "sg_test_rwbuf"
	sg_test_rwbuf $device_sg 10 >$stdout 2>$stderr
	tc_pass_or_fail $? "sg_test_rwbuf $device_sg" 
}
#
#TC_sg_turs
#
function TC_sg_turs()
{
	if [ -z "$device_sg" ] ; then
		tc_info "No SCSI DISK maps as SG deivce,SKIP sg_trus test!"
		return
	fi
	tc_register "sg_turs"
	sg_turs -n=1000 $device_sg >$stdout 2>$stderr
	tc_fail_if_bad $? "sg_sg_turs -n=1000 $device_sg" 
	sg_turs -n=1000 -t $device_sg >$stdout 2>$stderr
	grep -q "time to perform commands was" $stdout
	tc_pass_or_fail $? "sg_sg_turs -t $device_sg" 
}
#
#
#TC_sgm_dd
#
function TC_sgm_dd()
{
	if [ -z "$device_sg" ] ; then
		tc_info "No SCSI DISK maps as SG deivce,SKIP sgm_dd test!"
		return
	fi
	tc_register "sgm_dd"
	sgm_dd if=$device_sg of=$TCTMP/sg.dat bs=512 count=45 >$stdout 2>&1
	grep -q "45+0 records out" $stdout 
	tc_pass_or_fail $? "sgm_dd: $stdout"
}
#
#
#TC_sgp_dd
#
function TC_sgp_dd()
{
	if [ -z "$device_sd" ] ; then
		tc_info "No SCSI DISK maps as SG deivce,SKIP sgp_dd test!"
		return
	fi
	tc_register "sgp_dd"
	sgp_dd if=$device_sg of=$TCTMP/sg.dat bs=512 count=45 >$stdout 2>&1
	grep -q "45+0 records out" $stdout 
	tc_pass_or_fail $? "sgp_dd: $stdout"
}

# 
# main
# 

#TST_TOTAL=1
tc_setup
tc_root_or_break || exit
tc_executes line grep cut || exit
build_host=`uname -m`
# if no sg driver exists, skip the test.
device_sg=`sg_scan -n | line |cut -d ":" -f 1`

[ -n "$device_sg" ] 
tc_break_if_bad $? "No sg device to run the test!" || exit
tc_info "USE $device_sg as the test device!"
[ "$build_host" = "ppc64" ] || commands="$commands $commands_ext"
for cmd in $commands
do
	tc_register "$cmd"
	tc_executes $cmd
	tc_fail_if_bad $? "$cmd not installed" || continue
	TC_$cmd
done

