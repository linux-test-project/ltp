#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2003						      ##
##                                                                            ##
## This program is free software;  you can redistribute it and or modify      ##
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
#
# File :       util_linux_sh
#
# Description: This testcase tests a collection of 69 basic system utilities:
#               
# adjtimex	agetty		arch		blockdev 	cal
# cfdisk	chkdupexe	clock		col		colcrt
# colrm		column		ctrlaltdel	cytune		ddate
# dmesg		elvtune		fdformat	fdisk		flushb
# fsck.minix	getopt		guessfstype	hexdump		hostid
# hwclock	ipcrm		ipcs		isosize		kill
# line		logger		look		losetup		mcookie
# mesg		mkfs		mkfs.bfs	mkfs.minix	mkswap
# more		mount		namei		nologin		pivot_root
# ramsize	raw		rcraw		rdev		readprofile
# rename	renice		rev		rootflags	script
# setfdprm	setsid		setterm		sfdisk		sln
# swapoff	swapon		tunelp		ul		umount
# vidmode	wall		whereis		write
#               
# Author:       Andrew Pham, apham@us.ibm.com
#
# History:      Jan 29 2003 - Created -Andrew Pham.
#		Feb 24 2003 - Added more testcases and modularized all testcases
#		Mar 07 2003 - Stubbed all the unimplemented tests and modified 
#			all tests to use tc_utils.source functionalities.
#		Mar 27 2003 - Fixed main section to run on PPC too.
#		Apr 22 2003 - Fixed TC_swapon and TC_swapoff to work on a
#			busybox system.
#		07 Jan 2004 - (apham) updated to tc_utils.source
#		24 Feb 2004 (rcp) fix TC_col to not hang
#		05 May 2004 (rcp) general cleanup -- much more to do
#		06 May 2004 (rcp) improved man page usage.
#
################################################################################
commands=" adjtimex agetty arch blockdev cal cfdisk chkdupexe col \
	colcrt colrm column ctrlaltdel cytune ddate dmesg elvtune \
	fdformat fdisk flushb fsck.minix getopt guessfstype \
	hexdump hostid hwclock clock ipcrm ipcs isosize kill line \
	logger look losetup mcookie mesg mkfs mkfs.bfs mkfs.minix \
	mkswap more mount namei nologin pivot_root ramsize raw \
	rcraw rdev readprofile rename renice rev rootflags \
	script setfdprm setsid setterm sfdisk sln \
	swapoff swapon tunelp ul umount vidmode wall whereis write "

# NOTE: In the above clock must come after hwclock

# source the utility functions 
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TST_TOTAL=0
for i in $commands ; 
do
         let TST_TOTAL+=1
done

# Initialize output messages
summary2=": Failed: Not available."
summary3=": Failed: Unexpected output."

FIVCMD=fivcmd$$
MANCACHE=/var/cache/man
MANSHARE=/usr/share/man

################################################################################
# utility functions
###############################################################################

function tc_local_setup()
{
	# put a manpage in place
	cp tmanpage.150x.gz $MANSHARE/man1/$FIVCMD.1.gz
	chmod a+r $MANSHARE/man1/$FIVCMD.1.gz

	# save original index.db
	mv $MANCACHE/index.db $MANCACHE/index.db-$$

	# save original /etc/manpath.config
	mv /etc/manpath.config /etc/manpath.config-$$
	cp /etc/manpath.config-$$ /etc/manpath.config
}

function tc_local_cleanup()
{
	# restore saved files
	[ -f  /etc/manpath.config-$$ ] && mv /etc/manpath.config-$$ /etc/manpath.config
	[ -f $MANCACHE/index.db-$$ ] && mv $MANCACHE/index.db-$$ $MANCACHE/index.db

	# remove generated files
	rm -f $MANSHARE/man1/$FIVCMD.1.gz
	rm -f $MANCACHE/cat1/$FIVCMD.1.gz
	#make sure umount stuffs
	umount $TCTMP/mnt_pt >&/dev/null
}
#
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
# a flag to use in check_adj and rt_adj
#
adj_file=0

#
# check the system CMOS clock conf file for system restoration later
#
function check_adj()
{
	if [ -s /etc/adjtime ]; then
		adj_file=1
		mv /etc/adjtime /etc/adjtime.orig
	fi
}

#
# restore the system CMOS clock configuration back to its original state.
#
function rt_adj()
{
	if [ $adj_file -eq 1 ]; then
		mv /etc/adjtime.orig /etc/adjtime
	else
		rm /etc/adjtime
	fi
}

#
# Check to see if we are running on a busybox system.
#
function Is_Busybox()
{
	ls -l `which ls` | grep busybox >&/dev/null
	return $?
}
################################################################################
# the testcase functions
################################################################################

#
# test01
#
function TC_arch()
{
	TCNAME="arch - display system architecture."

	# Check to make sure that CPU is set
	[ "$CPU" ]
	tc_break_if_bad $? "arch: CPU is not set" || return

	arch >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from \"arch\""

	[ $CPU = "$(arch)" ]
	tc_pass_or_fail $? "expected to see \"$CPU\" in output"
}

#
# test2
#
function TC_cal()
{
	TCNAME="cal - displays calendar."
	
	cal >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return 

	cal | grep `date +%B`  &>/dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "cal -y: displays the whole year."
        let TST_TOTAL+=1

	cal -y >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return 

	local fail=0
	for month in January February March April May June July \
    	    	August September October November December
	do
		cal -y | grep $month &>/dev/null
		if [ $? -ne 0 ]; then
			fail=1
			break
		fi
	done

	tc_pass_or_fail $fail  "$summary3" 
	return 
}

#
#
# test3
#
function TC_chkdupexe()
{
	TCNAME="chkdupexe - find duplicate executables"
	local RC=0
	
	# creat some duplicate executables for testing
	MyPath03=$PATH

	mkdir $TCTMP/d1 &>/dev/null
	mkdir $TCTMP/d2 &>/dev/null
	cp $LTPBIN/util-linux.sh $TCTMP/d1/tstbin03 &>/dev/null
	cp $LTPBIN/util-linux.sh $TCTMP/d2/tstbin03 &>/dev/null

	export PATH=$PATH:$TCTMP/d1:$TCTMP/d2
	# actual test begins

	chkdupexe >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || RC=$? 
	
	if [ $RC -eq 0 ]; then
		chkdupexe | grep tstbin03 &>/dev/null 
		tc_pass_or_fail $?  "$summary3" || RC=$? 
	fi

	# reset PATH
	export PATH=$MyPath03
	return $RC
}

#
# test4
#
function TC_more()
{
	TCNAME="more - file perusal filter for crt viewing"

        echo "1: This is a testfile to test the" > $TCTMP/test_file04 
	echo "2: more command." >> $TCTMP/test_file04 
	echo "3: That's all folks!" >> $TCTMP/test_file04 
	echo "4: xyadkan" >> $TCTMP/test_file04

	more $TCTMP/test_file04 >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return 

	more $TCTMP/test_file04 | grep xyadkan &>/dev/null
	tc_pass_or_fail $?  "$summary3" || return 

	tc_register "more +num: display from line \"num\" on."
	let TST_TOTAL+=1

	more +2 $TCTMP/test_file04 >& /dev/null
	tc_fail_if_bad $? "summary2" || return 

	more +2 $TCTMP/test_file04 | grep -v testfile &>/dev/null
	tc_pass_or_fail $?  "$summary2" 
	return 
}

#
# test5
#

function TC_ddate()
{
	TCNAME="ddate - with no option."

	ddate &>/dev/null
	tc_pass_or_fail $?  "$summary2" || return 

	tc_register "ddate -format: ddate with the format flag."
	let TST_TOTAL+=1
	
	ddate +"%{%A %e %B%} %Y %NCelebrate %H" 7 2 2003 >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return 

	ddate +"%{%A %e %B%} %Y %NCelebrate %H" 7 2 2003 | grep 3169 &>/dev/null
	tc_pass_or_fail $?  "$summary3" 
	return 
}

#
# test6
#
function TC_hostid()
{
	hostid >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return 

	hostid | grep -q 0x
	tc_pass_or_fail $?  "$summary3" 
}

#
# test7
#
function TC_fdformat()
{
	test_manually
}

#
# test8
#
function TC_cfdisk()
{
	test_manually
}

#
# test9
#
function TC_colrm()
{
	echo 123456 | colrm >/dev/null 2>>$stderr
	tc_pass_or_fail $?  "$summary2" || return 

	tc_register "colrm n m : remove from col n to col m."
	let TST_TOTAL+=1

	echo 123456 | colrm 3 5 | grep 6 &>/dev/null
	tc_pass_or_fail $?  "$summary3"

	tc_register "colrm n : remove from col n to EOL."
	let TST_TOTAL+=1

	echo 123456 | colrm 3  | grep -v 6 &>/dev/null
	tc_pass_or_fail $?  "$summary2"
	return 
}

#
# test10
#
function TC_dmesg()
{
	dmesg >$TCTMP/dmesg.txt 2>>$stderr
	tc_pass_or_fail $?  "$summary2" || return 

	if [ -s $TCTMP/dmesg.txt ]; then
	
		tc_register "dmesg -c : clear the kernel ring buffer."
		let TST_TOTAL+=1
	
		tc_root_or_break || return

		dmesg -c >/dev/null 2>$stderr
		tc_fail_if_bad $?  "$summary2" || return 

		dmesg >$TCTMP/dmesg.txt2 2>>$stderr
		if [ -s $TCTMP/dmesg.txt2 ]; then
			tc_pass_or_fail 1 "$summary3"
			return 
		fi
	fi

	tc_pass_or_fail 0 "$summary3"
	return
}

#
# test11
#
function TC_fsck.minix()
{
	test_manually
}

#
# test12
#
# Variable below needed for clock
function TC_hwclock()
{
	hwclock_flag=0
	hwclock >$stdout  2>$stderr
	tc_fail_if_bad $? "unexpected response from \"hwclock\"" || return
	cp $stdout $TCTMP/hwclock.txt

	grep -q seconds $stdout
	tc_pass_or_fail $? "expected to see \"seconds\" in stdout" || return

	# Check if supporting utilities are available
	tc_exec_or_break cut awk || return

	tc_info "hwclock --set --date="
	hwclock --set --date="11/11/11 12:34:11" >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from \"hwclock --set --date\"" || return

	hwclock | grep -q 12 && hwclock | grep -q 34
	tc_fail_if_bad $? "expected to see \"12:34\" in nstdout" || return

	# Reset hardware clock to system clock
	hwclock -w >$stdout 2>>$stderr
	tc_pass_or_fail $? "unexpected response from \"hwclock -w\"" \
			"WARNING: SYSTEM CLOCK MAY BE MESSED UP!"

	hwclock_flag=1 	# OK to test clock comand
}

#
# test13
#

function TC_mesg()
{
	test_manually
	return

	#TCNAME="mesg y"
	
	#mesg y > /dev/null
	#tc_fail_if_bad $? "$summary2" || return 

	#mesg | grep y > /dev/null
	#tc_pass_or_fail $?  "$summary3" || return 

	#tc_register "mesg n"
	#let TST_TOTAL+=1
	
	#mesg n > /dev/null
	#if [ $? -ne 1 ]; then
	#	tc_fail_if_bad 1  "$summary2" 
	#	return 1
	#fi
	
	#mesg | grep n >& /dev/null
	#tc_pass_or_fail $?  "$summary3" || return 
	
	#return 0 
}

#
# test14
#

# Needed the below variable for rdev
rdev_flag=0

function TC_ramsize()
{
	if [ -s /boot/vmlinuz ]; then
		ramsize /boot/vmlinuz >/dev/null 2>>$stderr
		tc_fail_if_bad $?  "$summary2"
		[ $? -ne 0 ] && rdev_flag=1 && return 1
		
		/usr/sbin/ramsize /boot/vmlinuz | grep Ramsize >& /dev/null
		tc_pass_or_fail $?  "$summary3"
		[ $? -ne 0 ] && rdev_flag=1 && return 1

		return 0
	else
		tc_break_if_bad 1 "kernel image is not at /boot/vmlinuz."
		return
	fi
}

#
# test15
#
function TC_rename()
{
	# Make sure no existing files with same name.
	rm -f $TCTMP/fortesting*
	
	# Creating some test files.
	echo "" >$TCTMP/fortesting1
	echo "" >$TCTMP/fortesting2
	echo "" >$TCTMP/fortesting3
	
	rename for xyz $TCTMP/fortesting* >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return 1

	local a=0
	ls $TCTMP/xyztesting* | grep -c xyz | grep 3 >&/dev/null
	tc_pass_or_fail $?  "$summary3" || return 1
	
	return 0
}

#
# test16
#
function TC_setfdprm()
{
	TCNAME="setfdprm -n"
	local RC=0

	if [ ! -e /dev/fd1 ]; then
		tc_break_if_bad 1 "setfdprm: /dev/fd1 is not available for testing."
		return 1
	fi
	
	setfdprm -n /dev/fd1 >/dev/null 2>>$stderr
	tc_pass_or_fail $?  "$summary2" || RC=1

	tc_register "setfdprm -y"
	let TST_TOTAL+=1

	setfdprm -y /dev/fd1 >/dev/null 2>>$stderr
	tc_pass_or_fail $?  "$summary2" || RC=1

	tc_register "setfdprm -c"
	let TST_TOTAL+=1

	setfdprm -c /dev/fd1  >/dev/null 2>>$stderr
	tc_pass_or_fail $?  "$summary2" || RC=1
	
	if [ -s /etc/fdprm ]; then
		tc_register "setfdprm -p"
		let TST_TOTAL+=1

		setfdprm -p /dev/fd1 360/360 >/dev/null 2>>$stderr
		tc_pass_or_fail $?  "$summary2" || RC=1
	else
		tc_register "setfdprm -p"
		let TST_TOTAL+=1

		setfdprm -p /dev/fd1 360/360 720 9 2 40 0 0x2A 0x02 0xDF 0x50
		tc_pass_or_fail $?  "$summary2" || RC=1
	fi

	[ $RC -eq 1 ] && return 1
	return 0
}

#
# test17
#
function TC_swapoff()
{
	TCNAME="swapoff -a"
	tc_root_or_break || return 1
	
	swapoff -a  >/dev/null 2>$stderr
	tc_pass_or_fail $?  "$summary3" || return 1

	if ! Is_Busybox ; then
		tc_register "swapoff -h"
		let TST_TOTAL+=1

		swapoff -h >/dev/null 2>>$stderr
		tc_pass_or_fail $?  "$summary2" || return 1
	fi

	return 0
}
#
# test18
#
function TC_swapon()
{
	TCNAME="swapon -a"
	tc_root_or_break || return 1

	swapon -a  >/dev/null 2>$stderr
	tc_pass_or_fail $?  "$summary2" || return 1

	if ! Is_Busybox ; then
		tc_register "swapoff -h"
		let TST_TOTAL+=1

		swapon -h >/dev/null 2>>$stderr
		tc_pass_or_fail $?  "$summary2" || return 1
	fi

	if [ -s /proc/swaps ]; then
		tc_register "swapoff -s"
		let TST_TOTAL+=1

		# Check if supporting utilities are available
		tc_exec_or_break wc awk || return 1

		swapon -s  >$TCTMP/swapon.txt 2>$stderr
		tc_fail_if_bad $?  "$summary2" || return 1
		
		local xy=0
		xy=`wc -l $TCTMP/swapon.txt | awk '{print $1}'`
		if [ $xy -le 1 ]; then
			tc_pass_or_fail 1  "$summary3"
			return 1
		fi
	fi
	
	return 0
}

#
# test19
#
function TC_vidmode()
{
	if [ -s /boot/vmlinuz ]; then
		vidmode /boot/vmlinuz >/dev/null 2>>$stderr
		tc_fail_if_bad $?  "$summary2"
		[ $? -ne 0 ] && rdev_flag=1 && return 1
		
		vidmode /boot/vmlinuz | grep Video >& /dev/null
		tc_pass_or_fail $?  "$summary3"
		[ $? -ne 0 ] && rdev_flag=1 && return 1

		tc_register "vidmode -1"
		let TST_TOTAL+=1

		vidmode /boot/vmlinuz -1 >& /dev/null
		tc_pass_or_fail $?  "$summary2"
		[ $? -ne 0 ] && rdev_flag=1 && return 1

		return 0

	else
		tc_break_if_bad 1 "kernel image is not at /boot/vmlinuz."
		return 1
	fi
}

#
# test20
#
function TC_rootflags()
{
	if [ -s /boot/vmlinuz ]; then
		rootflags /boot/vmlinuz >/dev/null 2>>$stderr
		tc_pass_or_fail $?  "$summary2"
		[ $? -ne 0 ] && rdev_flag=1 && return 1
		
		tc_register "rootflags 1"
		let TST_TOTAL+=1

		rootflags /boot/vmlinuz 1 >& /dev/null
		tc_pass_or_fail $?  "$summary2"
		[ $? -ne 0 ] && rdev_flag=1 && return 1

		return 0
	else
		tc_break_if_bad 1 "kernel image is not at /boot/vmlinuz."
		return 1
	fi
}

#
# test21
#
function TC_rdev()
{
	tc_root_or_break || return

	rdev >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2"
	[ $? -ne 0 ] && return 1
		
	if [ -s /etc/mtab ]; then
		# Check if supporting utilities are available
		tc_exec_or_break cut || return

		local xy=""
		
		xy=`rdev | cut -f2 -d" "`
		if [ $xy != '/' ]; then
			tc_pass_or_fail 1  "$summary3"
			return 1
		fi
	else
		tc_break_if_bad 1 "Unable to verify rdev \
		works correctly: no \/etc\/mtab."
	fi

#	if [ "`arch`" != "i386" ]; then
#		tc_break_if_bad 1 "Further testing of rdev only \
#		works with i386 arch, have: `arch`"
#		return 1
#	fi
	
	if [ $rdev_flag -ne 0 ]; then
		tc_pass_or_fail 1 "-R or -r or -v $summary2"
		return 1
	fi

	return 0
}

#
# test22
#
function TC_adjtimex()
{
	TCNAME="adjtimex -p"

	tc_root_or_break || return

	local RC=0

	adjtimex -p >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return 1

	adjtimex -p | grep time_constant >& /dev/null
	tc_pass_or_fail $?  "$summary3" || RC=1

	# Check if supporting utilities are available
	tc_exec_or_break awk || return 1

	local old_tick=0
	local old_freq=0
	old_tick=`adjtimex -p | awk '/tick/{print $2}'`
	old_freq=`adjtimex -p | awk '/frequency/{print $2}'`

	tc_register "adjtimex -f"
	let TST_TOTAL+=1

	adjtimex -f -1234567 >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return 1

	adjtimex -p | grep "\-1234567" >& /dev/null
	tc_pass_or_fail $?  "$summary3" || RC=1

	tc_register "adjtimex --tick"
	let TST_TOTAL+=1

	adjtimex --tick 9999 >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return 1

	adjtimex -p | grep "9999" >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return 1

	# Reset tick and freq back to their original values
	adjtimex -f $old_freq --tick $old_tick >& /dev/null

	[ $RC -ne 0 ] && return 1
	return 0
}


#
# test23
#

function TC_agetty()
{
	test_manually
	return 1
}

#
# test24
#
function TC_blockdev()
{
	blockdev --report >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from \"blockdev --report\"" || return

	# Look for a device to test with
	local device=""
	while read a b c d e f dev 
	do 
		device=$dev
		[ -n "$device" ] && break
	done < <(blockdev --report | grep "/dev/")
	[ $device ]
	tc_break_if_bad $? "Unable to find block device" || return
	
	tc_info "blockdev --setro $device"
	blockdev --setro $device >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from \"blockdev --setro $device\"" || return
	blockdev --report $device >$stdout 2>$stderr
	grep -q ro $stdout
	tc_fail_if_bad $? "expected to see \"ro\"" || return

	tc_info "blockdev --setrw $device"
	blockdev --setrw $device >/dev/null 2>$stderr
	tc_fail_if_bad $? "unexpected response from \"blockdev --setrw $device\"" || return
	blockdev --report $device >$stdout 2>$stderr
	grep -q rw $stdout
	tc_pass_or_fail $? "expected to see \"rw\"" || return
}

#
# test25
#

function TC_col()
{
	# Check if supporting utilities are available
	tc_exec_or_break man || return

	local cmd="man $FIVCMD"
	echo "" | $cmd &>$stdout
	tc_fail_if_bad $? "\"$cmd\" failed" || return

	cp $stdout $TCTMP/manout
	cat $TCTMP/manout | col >$stdout 2>$stderr
	tc_fail_if_bad $? "col failed" || return

	cat $TCTMP/manout | col -b >$stdout 2>$stderr
	tc_fail_if_bad $? "col -b failed" || return

	grep -q NAME $stdout 2>$stderr
	tc_pass_or_fail $? "col -b | grep NAME failed"
}

#
# test26
#

function TC_colcrt()
{
	# Check if supporting utilities are available
	tc_exec_or_break nroff man || return

	local cmd="man $FIVCMD"
	echo "" | $cmd &>$stdout
	tc_fail_if_bad $? "\"$cmd\" failed" || return

	cp $stdout $TCTMP/manout
	cat $TCTMP/manout | col >$stdout 2>$stderr
	tc_fail_if_bad $? "col failed" || return

	cat $TCTMP/manout | nroff -ms | colcrt >$stdout 2>$stderr
	tc_fail_if_bad $? "colcrt failed" || return

	cat $TCTMP/manout | nroff -ms | colcrt - >$stdout 2>$stderr
	tc_fail_if_bad $? "colcrt - failed" || return

	grep -q '\-\-\-\-\-\-' $stdout 2>$stderr
	[ $? -ne 0 ]
	tc_fail_if_bad $? "grep found '\-\-\-\-\-\-'" || return

	cat $TCTMP/manout | nroff -ms | colcrt -2 >$stdout 2>$stderr
	tc_pass_or_fail $? "colcrt -2 failed" || return
}

#
# test27
#

function TC_column()
{
	# Check if supporting utilities are available
	tc_exec_or_break wc awk || return 
	
	# Generate a test file
	echo "1234567890" > $TCTMP/column.tst
	echo "1234567890" >> $TCTMP/column.tst
	
	column $TCTMP/column.tst > $TCTMP/column.res 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return 1
	
	local line=0
	line=`wc -l $TCTMP/column.res | awk '{print $1}'`

	if [ $line -gt 1 ]; then
		tc_pass_or_fail 1  "$summary3"
		return 1
	else
		tc_pass_or_fail 0  "passed."
	fi

	let TST_TOTAL+=1
	let TST_COUNT+=1

	column -t -s5 $TCTMP/column.tst > $TCTMP/column.res1 2>$stderr
	tc_fail_if_bad $?  "-t -s $summary2" || return 1
	
	local word=0
	word=`wc -w $TCTMP/column.res1 | awk '{print $1}'`
	if [ $word -le 2 ]; then 
		tc_pass_or_fail 1  "-t -s $summary3"
		return 1
	else
		tc_pass_or_fail 0  "-t -s : passed."
	fi
	
	return 0
}

#
# test28
#
function TC_ctrlaltdel()
{
	test_manually
	return 1
}

#
# test29
#
function TC_cytune()
{
	tc_break_if_bad 1 "$TCNAME must be tested manually: need a Cyclades card."
	return 1
}

#
# test30
#

function TC_elvtune()
{
	ls /dev/blkdev* >& /dev/null
	tc_break_if_bad $? "Unable to test $TCNAME:/dev/blkdev* not exist." || return

	# block device available, pick one for testing.

	local device=""
	local i=1
	while [ $i -le 5 ]
	do
		ls /dev/blkdev$i >& /dev/null
		if [ $? -eq 0 ]; then
			device=/dev/blkdev$i
			break
		fi
		i=$(($i+1))
	done
	
	elvtune -r 10 $device >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "-r $summary2" || return

	elvtune -w 10 $device >/dev/null 2>>$stderr
	tc_pass_or_fail $?  "-w $summary2" || return
}

#
# test31
#

function TC_fdisk()
{
	tc_info "fdisk: Some features must be tested manually."
	tc_info "fdisk: Refer to the manual test section."

	tc_root_or_break || return
	
	fdisk -v >$stdout 2>$stderr
	tc_fail_if_bad $?  "-v $summary2" || return 

	[ -s $stdout ]
	tc_pass_or_fail $?  "-v $summary3" || return 
}

#
# test32
#

function TC_flushb()
{
	# Check if supporting utilities are available
	tc_exec_or_break awk || return

	# Look for a device to test with
	local device=""
	while read a b c d e f dev 
	do 
		device=$dev
		[ -n "$device" ] && break
	done < <(blockdev --report | grep "/dev/")
	[ $device ]
	tc_break_if_bad $? "Unable to find block device" || return

	flushb $device >$stdout 2>$stderr
	tc_pass_or_fail $? "unexpected response from \"flushb $device\""
}

#
# test33
#
function TC_clock()
{
	tc_root_or_break || return

	uname -m | grep -q "ppc"  && {
		let TST_TOTAL--
		tc_info "$TCNAME: skip this test on PPC systems"
		return 0
	}

	clock >$stdout 2>$stderr
	tc_fail_if_bad $? "$summary2" || return

	[ $hwclock_flag -eq 1 ]
	tc_break_if_bad $? "because hwclock failed." || return

	tc_pass_or_fail 0 "paassed if we get this far"
}

#
# test34
#

# Helper function to test34
function TC_getopt_h()
{
	local RC=0
	TEMP=`getopt -o ab:c:: --long a-long,b-long:,c-long:: \
	     -n 'example.bash' -- "$@"`
	if [ $? != 0 ] ; then 
		tc_pass_or_fail 1 "$summary2"
		return 1 
	fi

	# Note the quotes around `$TEMP': they are essential!
	eval set -- "$TEMP"

	while true ; do
		case "$1" in
			-a|--a-long) shift
				     TCNAME="getopt -o"
				     tc_pass_or_fail 0 "";;
			-b|--b-long) tc_register "getopt - with a required argument."
				     let TST_TOTAL+=1
				     if [ $2 != "very_long" ]; then
						tc_pass_or_fail 1 "$summary3"
						RC=1
			 	     else
						tc_pass_or_fail 0 ""
				     fi
				shift 2;;
			-c|--c-long)
			# c has an optional argument. As we are in quoted mode,
			# an empty parameter will be generated if its optional
			# argument is not found.
				case "$2" in
					"") tc_register "getop- with empty optional arg."
					    let TST_TOTAL+=1
					     tc_pass_or_fail 0 ""
					    shift 2;;
					*) tc_register "getop- with optional arg."
					    let TST_TOTAL+=1
					    if [ "$2" != "more" ]; then
						tc_pass_or_fail 1 "incorrect optional argument."
					
						RC=1
					    else
						tc_pass_or_fail 0 ""
					    fi
						shift 2;;
				esac ;;
			--) shift ; break ;;
			*) return 1 ;;
		 esac
	done
	local nopar=0
	for arg do 
		if ! [ "$arg" == par1 -o "$arg" == par2 -o "$arg" == par3 ]
		then
			nopar=1
			break
		fi
	done
	
	tc_register "getopt - with no-optional arguments."
	let TST_TOTAL+=1
	
	if [ $nopar -ne 0 ]; then
		tc_pass_or_fail 1  "no-option parameters are incorrect"
		return 1
	else
		tc_pass_or_fail 0 "no-option parameters are correct."
	fi

	[ $RC -ne 0 ] && return 1
	return 0
}

function TC_getopt()
{
	TCNAME="getopt -o"
	local x=0

	TC_getopt_h -a par1 par2 --c-long par3 -cmore -b "very_long"
	return $?
}

#
# test35
#
function TC_guessfstype()
{
	guessfstype /dev/had >/dev/null 2>>$stderr
	tc_pass_or_fail $?  "$summary2" || return 1

	return 0
}

#
# test36
#
function TC_hexdump()
{
	echo "1 2 3" > $TCTMP/hexdump.txt
	echo "4 5 6" >> $TCTMP/hexdump.txt
	
	hexdump $TCTMP/hexdump.txt >$stdout 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return 1
	
	[ -s $stdout ]
	tc_pass_or_fail $?  "$summary3" || return 
	
	tc_register "hexdump -c"
	let TST_TOTAL+=1

	hexdump -c $TCTMP/hexdump.txt >$stdout 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return 
	
	hexdump -c $TCTMP/hexdump.txt | grep \\n >& /dev/null
	tc_pass_or_fail $?  "-c $summary3" || return 

	tc_register "hexdump -s"
	let TST_TOTAL+=1

	hexdump -s 2 $TCTMP/hexdump.txt >$stdout 2>$stderr
	tc_fail_if_bad $?  "-s $summary2" || return 
	
	[ -s $stdout ]
	tc_pass_or_fail $?  "-s $summary2"
}

#
# test37
#
function TC_ipcrm()
{
	# Check if supporting utilities are available
	which sem_ipcrm >& /dev/null
	if [ $? -ne 0 ]; then
		tc_break_if_bad 1 "Make sure you did: make install \
		and PATH is set correctly."
		return 1
	fi
	
	# Check if supporting utilities are available
	tc_exec_or_break ipcs || return

	local id=12123210
	id=`sem_ipcrm`
	
	if [ -z "$id" -o $id -eq 12123210 ]; then
		tc_break_if_bad 1 "Unable to create a semaphore to test further."
		return 1
	fi
	
	ipcrm sem $id > /dev/null 2>>$stderr
	tc_fail_if_bad $?  "sem $summary2" || return

	ipcs | grep -v $id >& /dev/null
	tc_pass_or_fail $?  "sem $summary3"
}

#
# test38
#
function TC_ipcs()
{
	ipcs >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return 
	
	ipcs | grep Semaphore >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return 

	tc_register "ipcs -q"
	let TST_TOTAL+=1

	ipcs -q >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return 
	
	ipcs -q | grep -v Semaphore >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return 

	tc_register "ipcs -m"
	let TST_TOTAL+=1

	ipcs -m >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return 
	
	ipcs -m | grep -v Semaphore >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}
#
# test39
#
function trapping39()
{
	trap "flag39=1" 14
}
# For use int test39
flag39=0

function TC_kill()
{
	# Check if supporting utilities are available
	tc_exec_or_break ps pgrep || return 

	local pid=0
	sleep 30m & >& /dev/null
	pid=`pgrep -n`
	if [ -z $pid -o $pid -eq 0 ]; then
		tc_break_if_bad 1 "Unable to get pid of \'sleep 30m \'." 
		return 
	fi
	
	kill $pid >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return 
	
	sleep 2 
	
	ps -ef > $TCTMP/kill.tst 2>/dev/null
	cat $TCTMP/kill.tst | grep -q $pid
	[ $? -ne 0 ]
	tc_pass_or_fail $?  "$summary3" || return 
	
	tc_register "kill -s"
        let TST_TOTAL+=1
	
	# Call function trapping 39 to trap a sent signal, 14
	trapping39
	
	pid=`pgrep -n`
	kill -s 14 $pid >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return 1

	sleep 3

	if [ $flag39 -eq 0 ]; then
		tc_pass_or_fail 1 " $summary3"
		return 1
	else
		tc_pass_or_fail 0 ""
	fi
}

#
# to recreate the test data:
#	1) mkdir test
#	2) echo "for testing one">test/f1
#	3) echo "for testing two">test/f2
#	4) echo "for testing three">test/f3
#	5) mkisofs -R -o isosize.data test
#
function TC_isosize()
{
	# Check if supporting utilities are available
	tc_exec_or_break  mkisofs || return 

	mkdir $LTPTMP/test_$$
	echo "for testing one" > $LTPTMP/test_$$/f1
	echo "for testing two" > $LTPTMP/test_$$/f2
	echo "for testing three" > $LTPTMP/test_$$/f3
	mkisofs -R -o $LTPBIN/isosize.data $LTPTMP/test_$$

	if [ ! -s $LTPBIN/isosize.data ]; then
		tc_break_if_bad 1 "The datafile $LTPBIN/isosize.data is not found"
		return 1
	fi

	isosize $LTPBIN/isosize.data >$stdout 2>$stderr
	tc_fail_if_bad $? "$summary2" || return 

	isosize $LTPBIN/isosize.data | grep 98304 >&/dev/null
	tc_pass_or_fail $? "Expected size: 98304, got: $stdout" || return

	tc_register "isosize -x"
	let TST_TOTAL+=1
	
	 isosize -x $LTPBIN/isosize.data >$stdout 2>$stderr
	 tc_fail_if_bad $? "$summary2" || return 1

	local expected="sector count: 48, sector size: 2048"
	local result=`isosize -x $LTPBIN/isosize.data`	
	[ "$expected" = "$result" ]
	tc_pass_or_fail $? "Expected : $expected, Got: $result"
}

# Procedure for doing manual test:
#	1) Make sure there are at least 2 users logged on the system, e.g.
#		look at output of : 'ps -ef'
#	2) Make sure both terminals have write permission turn on, e.g.
#		issue the following command at each terminal: 'mesg y'
#	3) One of the users should inniate the conversation by issue:
#		'write <otheruser name> '
#	4) If data transmit and receive correctly, then the test is passed.
#		otherwise failed.
#	5) terminate the test by: ' Ctrl-D'

function TC_write()
{
	test_manually
	return 1
}
function TC_ul()
{
	echo "one -" >> $TCTMP/ul_tst
	echo "two" >> $TCTMP/ul_tst
	echo "t_h_r_e_e" >> $TCTMP/ul_tst

	ul $TCTMP/ul_tst >$stdout 2>$stderr
	tc_pass_or_fail $? "$summary2"
}

#
# test
#
function TC_whereis()
{
	TCNAME="whereis -u -M"
	
	mkdir $TCTMP/D1 $TCTMP/D2 $TCTMP/D3 >& /dev/null

	touch $TCTMP/D1/ff $TCTMP/D1/gg $TCTMP/D2/ff.gz $TCTMP/D3/gg >&/dev/null
	cd $TCTMP/D1
	whereis -u -M $TCTMP/D2/ -S $TCTMP/D3 -f * >& /dev/null
	tc_fail_if_bad $?  "-uMSf $summary2" || return

	whereis -u -M $TCTMP/D2/ -S $TCTMP/D3 -f * | grep "$TCTMP\/D3\/gg" \
	>& /dev/null
	tc_pass_or_fail $?  "-uMSf $summary3"
}

function TC_logger()
{
	# Make syslogd is running	
	if ! /etc/init.d/syslog restart >&/dev/null ; then
		tc_info "Unable to restart syslogd."
		return 1
	fi

	local mesg1="Testing message for logger xyz$$zyx."
	logger $mesg1 >$stdout 2>$stderr
	tc_fail_if_bad $? "$summary2" || return

	sleep 1

	grep -q "xyz$$zyx" /var/log/messages
	tc_pass_or_fail $? "Expected: $mesg1 in /var/log/messages" || return

	tc_register "logger -t -f"
	let TST_TOTAL+=1
	local TAG="2xyz$$zyx2"

	echo "Testing file for logger." >$TCTMP/logger_tst
	echo "two" >>$TCTMP/logger_tst
	echo "three" >>$TCTMP/logger_tst

	logger -t $TAG -f $TCTMP/logger_tst >$stdout 2>$stderr
	tc_fail_if_bad $? "$summary2" || return

	sleep 2
	grep -c $TAG  /var/log/messages | grep -q 3
	tc_pass_or_fail $? "$summary"
}

function TC_look()
{
	echo "is dkfjalkdjfa" >$TCTMP/look.txt
	echo "this dkfjakdfk" >>$TCTMP/look.txt
	echo "the akdjfkjajf" >>$TCTMP/look.txt
	echo "what xyz123" >>$TCTMP/look.txt
	echo "whatever abc456" >>$TCTMP/look.txt

	if [ ! -s $TCTMP/look.txt ]; then
		tc_break_if_bad 1 "Unable to create a test file: $TCTMP/look.txt"
		return
	fi
	
	look whatever $TCTMP/look.txt >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return
	
	look whatever $TCTMP/look.txt | grep -q abc456
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "look -t"
	let TST_TOTAL+=1
	
	look -t t whatever $TCTMP/look.txt  > /dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return 
	
	look -t t whatever $TCTMP/look.txt | grep -q xyz123
	tc_pass_or_fail $?  "-t $summary3"
}

# Procedure to run test46 manually:
# 	1) create a test file, via: 
#	2) issue: "wall <testfile>" as root
#	3) if the content of the <testfile> show up all users that logged in,
#	   the test passed.  Otherwise it failed.
function TC_wall()
{
	test_manually
}

function TC_raw()
{
	unimplemented 
}

function TC_sln()
{
	echo "1 2 3 myxyz" >$TCTMP/sln.txt
	echo "4 5 6" >>$TCTMP/sln.txt
	
	sln $TCTMP/sln.txt $TCTMP/sln123 >$stdout 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return 
	
	grep -q myxyz $TCTMP/sln123
	tc_fail_if_bad $?  "$summary3" || return 

	echo whatisthis > $TCTMP/sln123 2> /dev/null
	grep whatisthis $TCTMP/sln.txt >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

function TC_script()
{
	test_manually
}

function TC_mkfs()
{
	unimplemented 
}

function TC_mkfs.bfs()
{
	unimplemented 
}

function TC_mount()
{
	# Check if supporting utilities are available
	tc_exec_or_break gunzip mkdir || return 

	#Create a mount point and mount a fs
	mkdir $TCTMP/mnt_pt || return

	cp $LTPBIN/ul_mnt.img.gz $TCTMP
	gunzip $TCTMP/ul_mnt.img.gz
	mount $TCTMP/ul_mnt.img $TCTMP/mnt_pt -o loop >$stdout 2>$stderr
	tc_fail_if_bad $? "$summary2" || return

	grep -q successfully $TCTMP/mnt_pt/status
	tc_pass_or_fail $? "$summary3"
}

function TC_renice()
{
	unimplemented 
}

function TC_setsid()
{
	unimplemented 
}

function TC_namei()
{
	unimplemented 
}

function TC_rcraw()
{
	unimplemented 
}

function TC_rev()
{
	unimplemented 
}

function TC_setterm()
{
	unimplemented 
}

function TC_tunelp()
{
	unimplemented 
}

function TC_losetup()
{
	unimplemented 
}

function TC_nologin()
{
	unimplemented 
}

function TC_sfdisk()
{
	unimplemented 
}

function TC_mcookie()
{
	unimplemented 
}

function TC_mkswap()
{
	unimplemented 
}

function TC_pivot_root()
{
	unimplemented 
}

function TC_readprofile()
{
	unimplemented 
}

function TC_umount()
{
	if ! mount | grep -q $TCTMP/mnt_pt ; then
		tc_break_if_bad 1 "Nothing mounted to umount"
		return
	fi
	
	umount $TCTMP/mnt_pt >$stdout 2>$stderr
	tc_fail_if_bad $? "$summary2" || return

	mount | grep -q $TCTMP/mnt_pt
	[ $? -ne 0 ]
	tc_pass_or_fail $? "$summary3"
}

function TC_line()
{
	echo "Line number one in my testing file xyzxxyyzz." >$TCTMP/line_tst.txt
	echo "Line number two in my testing file." >>$TCTMP/line_tst.txt
	
	line < $TCTMP/line_tst.txt > $stdout 2>$stderr
	tc_fail_if_bad $? "$summary2" || return

	grep -q xyzxxyyzz $stdout 
	tc_pass_or_fail $? "$summary3"
}

function TC_mkfs.minix()
{
	unimplemented 
}
################################################################################
# main
################################################################################
tc_setup
check_adj

# Check if supporting utilities are available
tc_exec_or_break sleep date cp grep mkdir || return 

TST_TOTAL=1

for cmd in $commands 
do
	[ "$cmd" = "ramsize" -o "$cmd" = "rdev" -o "$cmd" =  "rootflags" -o "$cmd" = "vidmode" ] &&
	uname -m | grep -qv "i\?86" && {
       		let TST_TOTAL--
		tc_info "$cmd: skip this test on non x86 systems"
		continue
		}	
	tc_register "$cmd"
	tc_executes $cmd
	tc_fail_if_bad $? "$cmd not installed" || continue
	TC_$cmd
done
