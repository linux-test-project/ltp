#!/bin/bash
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
# File :        ps.sh
#
# Description:  This testcase tests the commands in the ps package. 
#	free 	killall lsdev 	pgrep 	 pkill 	procinfo 
#	pstree 	skill 	snice 	socklist tload 	top 
#	uptime 	vmstat 	w watch fuser ps sysctl
#              
# 
# Author:       Andrew Pham, apham@us.ibm.com
#
# History:      Feb 06 2003 - Created - Andrew Pham.
#  		Apr 17 2003 - fixed bug in testcases pstree -p and top -bnp.
#			Also changed all testcases to use tc_utils.source.
#			Andrew Pham
#		Apr 22 2003 - Fixed a typo: passfaill -> tc_pass_or_fail
#			Andrew Pham
#		Jul 29 2003 - Fixed sysctl tc to reset the hostname.
#			Andrew Pham
#		Oct 09 2003 - Fixed timming problem.
#		Oct 22 2003 - Fixed killall -e tc problem
#			    - Fixed unnecessary debug output in calculating
#				TST_TOTAL
#		07 Jan 2004 - (apham) updated to tc_utils.source
#		23 Jan 2004 (rcp) swapped test24 and test24xx since the
#				real test now works.
#		11 Feb 2004 (rcp) rewrote fuser test. BUG 6307
#		07 May 2004 (rcp) Rewrote fuser test again.
#				Added installation test.
#				Refuse to run against busybox version of ps.
#				Cleanup TST_TOTAL calculations.

# source the utility functions
me=`which $0`
LTPBIN=${me%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

# required supporting utilities
REQUIRED="ps grep cat chmod sleep wc kill" 

# commands in the ps package
commands=" ps free killall lsdev pgrep pkill procinfo \
         pstree skill snice socklist top uptime vmstat w ps sysctl"

################################################################################
# global variables
################################################################################

looper=""
myscript=""
myfile=""

# Initialize output messages
summary2="Not available."
summary3="Unexpected output."
################################################################################
# utility functions
################################################################################

#
# be sure none of my sleep commands are running
#
function no_sleep()
{
	ps | grep sleep |
	while read pid garbage
	do
		kill -9 $pid &>/dev/null
	done
}

#
#	tc_local_cleanup
#
function tc_local_cleanup()
{
	[ "$looper" ] && killall $looper &>/dev/null
	no_sleep
}

################################################################################
# the testcase functions
################################################################################

function test00()
{
	tc_register "installation check"
	tc_executes $commands
	tc_pass_or_fail $? "ps package not installed properly"
}

function test1()
{
	tc_register "ps"
	
	ps >$stdout 2>$stderr 
	tc_fail_if_bad $?  "$summary2" || return
	
	no_sleep
	
	sleep 22m &
	sleep 1
	ps > $TCTMP/res001
	grep -q sleep $TCTMP/res001
	tc_pass_or_fail $?  "$summary3"
}

function test2()
{
	tc_register "ps -e"

	ps > $TCTMP/tmp_ps
	ps -e > $TCTMP/tmp_pse 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return
	
	#check to see that ps -e output more stuffs than ps
	set `wc -l $TCTMP/tmp_pse`
	pse_l=$1
	set `wc -l $TCTMP/tmp_ps`
	ps_l=$1

	[ $pse_l -gt $ps_l ]
	tc_pass_or_fail $? " Failed: Unexpected output ."
}

function test3()
{
	tc_register "ps -f"
	
	ps > $TCTMP/tmp_ps 2>/dev/null
	ps -f > $TCTMP/tmp_psf 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	#check to see that ps -f output more stuffs than ps
	set `wc -w $TCTMP/tmp_psf`
	psf_w=$1
	set `wc -w $TCTMP/tmp_ps`
	ps_w=$1

	[ $psf_w -gt $ps_w ]
	tc_pass_or_fail $? "$summary3"
}

function test4()
{
	tc_register "free"
	local RC=0

	free >$stdout 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return
	
	for O in V b k m o
	do
		free -$O >$stdout 2>$stderr
		tc_fail_if_bad $?  "free -$O : Not work." || return
		if [ ! -s $stdout ]; then
			RC=1
			break
		fi
	done

	[ "$RC" -eq 0 ]
	tc_pass_or_fail $? "free -$O : $summary3"
}

function test5()
{
	tc_register "free -t"

	free -t >$stdout 2>$stderr 
	tc_fail_if_bad $?  "$summary2" || return

	grep -q Total $stdout
	tc_pass_or_fail $?  "$summary3"
}

function test6()
{
	tc_register "killall -l"
	
	killall -l >$stdout 2>$stderr 
	tc_fail_if_bad $?  "$summary2" || return

	grep -q HUP $stdout
	tc_pass_or_fail $?  "$summary3"
}

function test9()
{
	tc_register "killall -e"

	sleep 234m &
	local tmpPID=`pgrep -n sleep`
	
	sleep 2

	killall -e sleep >$stdout 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return 

	sleep 2
	ps --pid $tmpPID >&/dev/null
	[ ! $? -eq 0 ]
	tc_pass_or_fail $? "$summary3: NOT expect to see proc $tmpPID: `ps --pid $tmpPID`" 
}

function test8()
{
	tc_register "lsdev"

	lsdev  >$stdout 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	[ -s $stdout ]
	tc_pass_or_fail $?  "$summary3"
}

function test7()
{
	tc_register "pgrep -n"

	sleep 50m &
	pgrep -n >$stdout 2>$stderr
	tc_fail_if_bad $?  " $summary2" || return
	sleep 2

	local tmpPID=`cat $stdout`

	ps -ef | grep -q $tmpPID
	tc_pass_or_fail $?  "$summary3" || return
	
	tc_register "pgrep -l"
	let TST_TOTAL+=1
	pgrep -l sleep >$stdout 2>$stderr
	tc_fail_if_bad $?  " $summary2" || return

	grep -q sleep $stdout
	tc_pass_or_fail $?  "$summary3" || return

	if [ -n "$UID" ]; then
		tc_register "pgrep -U"
		let TST_TOTAL+=1
		pgrep -U $UID sleep >$stdout 2>$stderr
		tc_fail_if_bad $?  " $summary2" || return
		
		#cat $stdout | grep $tmpPID  # $stdout
		grep -q $tmpPID  $stdout
		tc_pass_or_fail $?  "$summary3"
	fi
}


function test10()
{
	tc_register "pkill"

	sleep 234m &
	local tmpPID=`pgrep -n sleep`
	sleep 1
	pkill sleep >$stdout 2>$stderr 
	tc_fail_if_bad $?  "$summary2" || return
	sleep 2

	ps -ef > $TCTMP/ps_out
	grep -q $tmpPID $TCTMP/ps_out
	[ $? -ne 0 ]
	tc_pass_or_fail $? "$summary3" 
}
function test11()
{
	tc_register "procinfo"

	procinfo -a >$stdout 2>$stderr
	tc_fail_if_bad $? "$summary2" || return

	[ -s $stdout ]
	tc_pass_or_fail $? "$summary3" 
}

function test12()
{
	tc_register "pstree"

	pstree >$TCTMP/pst_tmp 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return
  		
	[ -s $TCTMP/pst_tmp ]
	tc_pass_or_fail $? "$summary3"
}

function test13()
{
	tc_register "pstree -a"

	pstree -a >$TCTMP/pst_tmp_a
	tc_fail_if_bad $?  "$summary2" || return

	#check to see that pstree -a outputs more stuffs than pstree
	set `wc -l $TCTMP/pst_tmp_a`
	pst_a=$1
	set `wc -l $TCTMP/pst_tmp`
	pst=$1

	[ $pst_a -gt $pst ]
	tc_pass_or_fail $? "$summary3"
}

function test14()
{
	tc_register "pstree -p"

	pstree -p > $TCTMP/ps_p.tst 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	[ -s $TCTMP/ps_p.tst ]	
	tc_pass_or_fail $? "$summary3"
}

function test15()
{
	tc_register "skill -v"

	sleep 234m &
	sleep 2
	skill -v sleep >$stdout 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return
	
	[ -s $stdout ]
	tc_pass_or_fail $? "$summary3" || return

	# test skill -KILL
	tc_register "skill -KILL"
	let TST_TOTAL+=1

	sleep 235m &
	sleep 2
	skill  -KILL sleep >$stdout 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return 

	sleep 2

	ps -ef > $TCTMP/ps_out
	grep -q sleep $TCTMP/ps_out
	[ $? -ne 0 ]
	tc_pass_or_fail $? "$summary3"
}

function test16()
{
	tc_register "snice -v"

	sleep 234m &
	sleep 2

	# wait a bit for above process to kick in.
	local pid=`pgrep -n`
	local CNT=0

	snice -v  sleep >$stdout 2>$stderr 
	tc_fail_if_bad $?  " $summary2" || return

	[ -s $stdout ]
	tc_pass_or_fail $? " $summary3" || return

	tc_register "snice +17"
	let TST_TOTAL+=1

	snice +17 sleep >$stdout 2>$stderr 
	tc_pass_or_fail $?  "$summary2" || return

	# kill all sleep commands runing
	no_sleep	
	return 0
}
function test17()
{
	tc_register "socklist" 

	socklist >$stdout 2>$stderr 
	tc_fail_if_bad $?  " $summary2" || return

	grep -q inode $stdout
	tc_pass_or_fail $?  " $summary3"
}

function test18()
{
	tc_register "tload"
	tc_info "tload must be tested manually."
}

function test19()
{
	tc_register "top -bn"
	
	top -b -n 1 >$stdout 2>$stderr 
	tc_fail_if_bad $?  " $summary2" || return

	grep -q "1.*root.*init" $stdout
	tc_pass_or_fail $?  " $summary2"
}

function test20()
{
	tc_register "top -bnp"

	sleep 123m & >/dev/null
	local tmpPID=`pgrep -n sleep`
	sleep 2

	top -b -n 1  -p $tmpPID >$stdout 2>$stderr 
	tc_fail_if_bad $?  " $summary2" || return 

	grep -q $tmpPID $stdout
	tc_pass_or_fail $? " $summary3"
}

function test21()
{
	tc_register "uptime"

	uptime >$stdout 2>$stderr 
	tc_fail_if_bad $?  " $summary2" || return

	grep -q up $stdout
	tc_pass_or_fail $?  " $summary3"
}

function test22()
{
	tc_register "vmstat"

	sleep 234m &
	sleep 2
	vmstat >$stdout 2>$stderr 
	tc_fail_if_bad $?  " $summary2" || return

	grep -q cpu $stdout
	tc_pass_or_fail $?  " $summary3" || return

	tc_register "vmstat 1 5"
	let TST_TOTAL+=1

	vmstat 1 5 >$stdout 2>$stderr 
	tc_fail_if_bad $?  "$summary2" || return 

	set `wc -l $stdout`
	[ $1 -eq 7 ]
	tc_pass_or_fail $? "$summary3"
}

function test23()
{
	tc_register "w"

	w >$stdout 2>$stderr 
	tc_fail_if_bad $?  " $summary2" || return 

	grep -q JCPU $stdout
	tc_pass_or_fail $?  " $summary3" || return 

	tc_register "w -s"
	let TST_TOTAL+=1

	w -s >$stdout 2>$stderr 
	tc_fail_if_bad $?  " -s $summary2" || return 

	grep -q JCPU $stdout
	[ $? -ne 0 ]
	tc_pass_or_fail $? "$summary3"
}

function test24()
{
	tc_register "fuser"

	# create script that will loop forever
	# then redirect its output (of which there is none) to a file
	looper=looper$$.sh
	myscript=$TCTMP/$looper
	myfile=$TCTMP/myfile
	cat > $myscript <<-EOF
		#!$SHELL
		while : ; do
			sleep 1
		done
	EOF
	chmod +x $myscript
	$myscript > $myfile &		# will run until fuser -k
	local pid=$!			# pid of $myscript

	# wait for process to start
	local i=0;
	while [ $i -lt 10 ] ; do
		let i+=1
		ps -ef | grep -q "[l]ooper$$.sh" && break
		tc_info "waiting for $looper to start"
		sleep 1
	done
	ps -ef | grep -q "[l]ooper$$.sh"
	tc_break_if_bad $? "could not start a process for fuser -k to kill" || return

	# fuser should show pid for the file
	fuser $myfile >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response from fuser" || return
	grep -q "$pid" $stdout 2>$stderr
	tc_fail_if_bad $? "fuser didn't show PID $pid" || return

	tc_info "OK to see killed message for pid $pid"
	# should kill the $pid process
	fuser -k $myfile >$stdout 2>$stderr
	tc_fail_if_bad $? "Unexpected response from fuser -k"

	# wait for process to be killed
	local i=0;
	while [ $i -lt 10 ] ; do
		let i+=1
		ps -ef | grep -v grep | grep "$pid" >$stdout 2>$stderr
		[ -s $stdout ] || break
	done
	[ ! -s $stdout ]
	tc_pass_or_fail $? "process $pid was not killed by fuser -k"
	
}

function test25()
{
	tc_register "sysctl -a"
	local NEW="testing$$"
	local OLD=`hostname`

	# make sure we have root privilege
	tc_root_or_break || return

	sysctl -a >$stdout 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	grep -q 'kernel.hostname' $stdout
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "sysctl -w"
	let TST_TOTAL+=1

	sysctl -w kernel.hostname=$NEW >$stdout 2>$stderr 
	tc_fail_if_bad $?  "$summary2" || return

	grep -q $NEW $stdout
	tc_pass_or_fail $?  "$summary3" || return
	
	# resetting hostname
	sysctl -w kernel.hostname=$OLD >& /dev/null
	return 
}

function test26()
{
	tc_register "watch" 

	tc_info "watch must be tested manually."
}
################################################################################
# main
################################################################################
TST_TOTAL=27
ORIG_TST_TOTAL=$TST_TOTAL
tc_setup
tc_exec_or_break $REQUIRED || exit

# ensure not busybox version
! tc_is_busybox ps
tc_break_if_bad $? "Not supported for busybox version" || exit

test00 || exit	# installation check
i=1
while [ $i -lt $ORIG_TST_TOTAL ]
do
	test$i
	let i+=1
done
