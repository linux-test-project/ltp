#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :
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
#
# File :       textutils.sh
#
# Description: This testcase tests a collection of 28 basic text utilities:
#
# /bin/cat 		/bin/sort 		/usr/bin/cksum
# /usr/bin/comm 	/usr/bin/csplit 	/usr/bin/cut
# /usr/bin/expand 	/usr/bin/fmt 		/usr/bin/fold
# /usr/bin/head 	/usr/bin/join 		/usr/bin/md5sum
# /usr/bin/nl 		/usr/bin/od 		/usr/bin/paste
# /usr/bin/pr 		/usr/bin/ptx 		/usr/bin/sha1sum
# /usr/bin/sort 	/usr/bin/split 		/usr/bin/sum
# /usr/bin/tac 		/usr/bin/tail 		/usr/bin/tr
# /usr/bin/tsort 	/usr/bin/unexpand 	/usr/bin/uniq
# /usr/bin/wc
#               
#
# Author:       Andrew Pham, apham@us.ibm.com
#
# History:      Mar 10 2003 - Created -Andrew Pham.
#
#		07 Jan 2004 - (apham) updated to tc_utils.source
################################################################################
# all commands to be tested:

commands="cat comm expand head nl pr sort tsort wc csplit fmt \
	  join od split tail unexpand cksum cut fold md5sum \
	  paste sha1sum sum tr uniq ptx"

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

TST_TOTAL=0
for i in $commands
do
        let TST_TOTAL+=1
done

# Initialize output messages
summary2=": Failed: Not available."
summary3=": Failed: Unexpected output."

################################################################################
# utility functions
################################################################################

#
# report unimplemented test
# these are tests that should be implemented eventually
#
#       $1      any additional text to print
#
function unimplemented()
{
        tc_info "$TCNAME: not yet tested. $1"
}

# 
# test if a command is a busybox's command
#
function isbusybox()
{
	ls -l `which ls` | grep busybox >&/dev/null
}

#
# report a test must be done manually.
# procedure for how to do it is in each test heading.
#
function test_manually()
{
	tc_info "$TCNAME must be tested manually."
}

################################################################################
# the testcase functions
################################################################################

function TC_cat()
{

	# Check if supporting utilities are available
	tc_exec_or_break  echo grep || return
	
	echo "testing file for cat" >$TCTMP/cat.txt
	echo "line number 2 abcdefg" >>$TCTMP/cat.txt
	echo "line number 3 hijklmn" >>$TCTMP/cat.txt
	
	cat $TCTMP/cat.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	cat $TCTMP/cat.txt | grep hijklmn >&/dev/null
	tc_pass_or_fail $?  "$summary3" || return

	if ! isbusybox ; then
		tc_register "cat -n"
		let TST_TOTAL+=1

		cat -n $TCTMP/cat.txt >/dev/null 2>$stderr
		tc_fail_if_bad $?  "$summary2" || return

		cat -n $TCTMP/cat.txt | grep -v 4 >&/dev/null
		tc_pass_or_fail $?  "$summary3" || return 

		echo "" >>$TCTMP/cat.txt
		echo "" >>$TCTMP/cat.txt
		echo "" >>$TCTMP/cat.txt
		echo "" >>$TCTMP/cat.txt
		
		tc_register "cat -s"
		let TST_TOTAL+=1

		# Check if supporting utilities are available
		tc_exec_or_break  wc || return 
	
		cat -s $TCTMP/cat.txt > $TCTMP/cats.tst 2>$stderr
		tc_fail_if_bad $?  "$summary2" || return 

		wc -l $TCTMP/cats.tst | grep  4 >&/dev/null
		tc_pass_or_fail $?  "$summary3" || return 

	fi
}

function TC_comm()
{
	# Check if supporting utilities are available
	tc_exec_or_break  echo grep || return
	
	echo "1" >$TCTMP/comm1.txt
	echo "2" >>$TCTMP/comm1.txt
	echo "54321" >>$TCTMP/comm1.txt
	
	echo "1" >$TCTMP/comm2.txt
	echo "3" >>$TCTMP/comm2.txt
	echo "4" >>$TCTMP/comm2.txt
	echo "54321" >>$TCTMP/comm2.txt
	
	comm $TCTMP/comm1.txt $TCTMP/comm2.txt > /dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return 

	comm $TCTMP/comm1.txt $TCTMP/comm2.txt | grep 54321 >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return 

	tc_register "comm -1"
        let TST_TOTAL+=1

	comm -1 $TCTMP/comm1.txt $TCTMP/comm2.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return 

	comm -1 $TCTMP/comm1.txt $TCTMP/comm2.txt | grep -v 2 >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return 

	tc_register "comm -3"
        let TST_TOTAL+=1

	comm -3 $TCTMP/comm1.txt $TCTMP/comm2.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return 

	comm -1 $TCTMP/comm1.txt $TCTMP/comm2.txt | grep -v 54321 >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

function TC_expand()
{
	# Check if supporting utilities are available
	tc_exec_or_break  echo grep cat || return
	
	echo "1		a	" >$TCTMP/expand.txt
	echo "3		b	" >>$TCTMP/expand.txt
	echo "4		c	" >>$TCTMP/expand.txt
	echo "54321	d	" >>$TCTMP/expand.txt

	if isbusybox ; then
		expand $TCTMP/expand.txt > /dev/null 2>>$stderr
		tc_pass_or_fail $?  "$summary2" || return
	else
		expand $TCTMP/expand.txt > /dev/null 2>>$stderr
		tc_fail_if_bad $?  "$summary2" || return

		expand $TCTMP/expand.txt | cat -t | grep -v "\^I" >& /dev/null
		tc_pass_or_fail $?  "$summary3" || return

		tc_register "expand -i"
		let TST_TOTAL+=1

		expand -i $TCTMP/expand.txt > /dev/null 2>$stderr
		tc_fail_if_bad $?  "$summary2" || return

		expand -i $TCTMP/expand.txt | cat -t | grep "\^I" >& /dev/null
		tc_pass_or_fail $?  "$summary3" || return
	fi
}

function TC_head()
{
	# Check if supporting utilities are available
	tc_exec_or_break grep echo || return

        echo "1 xyz123" > $TCTMP/head1.txt
	echo "2" >> $TCTMP/head1.txt
	echo "3" >> $TCTMP/head1.txt
	echo "4" >> $TCTMP/head1.txt
	echo "5" >> $TCTMP/head1.txt
	
        echo "1 xyz123" > $TCTMP/head.txt
	echo "2" >> $TCTMP/head.txt
	echo "3" >> $TCTMP/head.txt
	echo "4" >> $TCTMP/head.txt
	echo "5" >> $TCTMP/head.txt
	echo "6" >> $TCTMP/head.txt
	echo "7" >> $TCTMP/head.txt
	echo "8" >> $TCTMP/head.txt
	echo "9" >> $TCTMP/head.txt
	echo "10" >> $TCTMP/head.txt

	head $TCTMP/head.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	head $TCTMP/head.txt | grep 10 >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "head -with 2 files."
        let TST_TOTAL+=1

	head $TCTMP/head.txt $TCTMP/head1.txt >/dev/null 2> $stderr
	tc_fail_if_bad $?  "$summary2" || return
	
	head $TCTMP/head.txt $TCTMP/head1.txt | grep -c xyz123 | grep 2 > /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "head -n"
        let TST_TOTAL+=1

	head -n 7 $TCTMP/head.txt >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	head -n 7 $TCTMP/head.txt | grep -v 10 >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

function TC_nl()
{
	# Check if supporting utilities are available
	tc_exec_or_break echo grep || return

        echo "1" > $TCTMP/nl.txt
	echo "2" >> $TCTMP/nl.txt
	echo "3" >> $TCTMP/nl.txt
	echo "4" >> $TCTMP/nl.txt
	echo "5" >> $TCTMP/nl.txt
	echo "6" >> $TCTMP/nl.txt
	echo "7" >> $TCTMP/nl.txt
	echo "8" >> $TCTMP/nl.txt
	echo "9" >> $TCTMP/nl.txt
	echo "ten" >> $TCTMP/nl.txt

	nl $TCTMP/nl.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	nl $TCTMP/nl.txt | grep 10 >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "nl -s" 
        let TST_TOTAL+=1
	
	nl -s abcde $TCTMP/nl.txt >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	nl -s abcde $TCTMP/nl.txt | grep abcde >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

function TC_pr()
{
	# Check if supporting utilities are available
	tc_exec_or_break echo grep || return

        echo "1" > $TCTMP/pr.txt
	echo "2" >> $TCTMP/pr.txt
	echo "3" >> $TCTMP/pr.txt
	echo "4" >> $TCTMP/pr.txt
	echo "5" >> $TCTMP/pr.txt
	echo "6" >> $TCTMP/pr.txt
	echo "7" >> $TCTMP/pr.txt
	echo "8" >> $TCTMP/pr.txt
	echo "9" >> $TCTMP/pr.txt
	echo "ten" >> $TCTMP/pr.txt

	cat $TCTMP/pr.txt > $TCTMP/pr1.txt
	cat $TCTMP/pr.txt >> $TCTMP/pr1.txt
	cat $TCTMP/pr.txt >> $TCTMP/pr1.txt
	cat $TCTMP/pr.txt >> $TCTMP/pr1.txt
	cat $TCTMP/pr.txt >> $TCTMP/pr1.txt
	cat $TCTMP/pr.txt >> $TCTMP/pr1.txt
	cat $TCTMP/pr.txt >> $TCTMP/pr1.txt
	
	pr $TCTMP/pr1.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	pr $TCTMP/pr1.txt | grep [Pp]age >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "pr -h" 
        let TST_TOTAL+=1
	
	pr -h MyTesting $TCTMP/pr1.txt >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	pr -h MyTesting $TCTMP/pr1.txt | grep MyTesting >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "pr -t" 
        let TST_TOTAL+=1
	
	pr -t $TCTMP/pr1.txt >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	pr -t $TCTMP/pr1.txt | grep -v [Pp]age >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

function TC_sort()
{
	# Check if supporting utilities are available
	tc_exec_or_break echo diff || return

        echo "1" > $TCTMP/sort.txt
	echo "3" >> $TCTMP/sort.txt
	echo "2" >> $TCTMP/sort.txt
	echo "c" >> $TCTMP/sort.txt
	echo "a" >> $TCTMP/sort.txt
	echo "b" >> $TCTMP/sort.txt

	sort $TCTMP/sort.txt > $TCTMP/sorted.tst 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

        echo "1" > $TCTMP/sort1.txt
	echo "2" >> $TCTMP/sort1.txt
	echo "3" >> $TCTMP/sort1.txt
	echo "a" >> $TCTMP/sort1.txt
	echo "b" >> $TCTMP/sort1.txt
	echo "c" >> $TCTMP/sort1.txt

	diff $TCTMP/sorted.tst $TCTMP/sort1.txt >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "sort -r" 
        let TST_TOTAL+=1
	
	sort -r $TCTMP/sort.txt > $TCTMP/sort_r.tst 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	echo "c" > $TCTMP/sorted_r.txt
	echo "b" >> $TCTMP/sorted_r.txt
	echo "a" >> $TCTMP/sorted_r.txt
	echo "3" >> $TCTMP/sorted_r.txt
	echo "2" >> $TCTMP/sorted_r.txt
        echo "1" >> $TCTMP/sorted_r.txt
	
	diff $TCTMP/sort_r.tst $TCTMP/sorted_r.txt >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "sort -c" 
        let TST_TOTAL+=1
	
	sort -c $TCTMP/sorted.tst > $TCTMP/sort_r.tst 2>>$stderr
	tc_pass_or_fail $?  "$summary3"
}

function TC_tac()
{
	# Check if supporting utilities are available
	tc_exec_or_break  echo diff || return
	
	echo "c" > $TCTMP/tac.txt
	echo "b" >> $TCTMP/tac.txt
	echo "a" >> $TCTMP/tac.txt
	echo "3" >> $TCTMP/tac.txt
	echo "2" >> $TCTMP/tac.txt
        echo "1" >> $TCTMP/tac.txt
	
	tac $TClTMP/tac.txt >$TCTMP/tac.tst l2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

        echo "1" > $TCTMP/tac.txt
	echo "2" >> $TCTMP/tac.txt
	echo "3" >> $TCTMP/tac.txt
	echo "a" >> $TCTMP/tac.txt
	echo "b" >> $TCTMP/tac.txt
	echo "c" >> $TCTMP/tac.txt
	
	diff $TCTMP/tac.txt $TCTMP/tac.tst >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}
function TC_tsort()
{
	# Check if supporting utilities are available
	tc_exec_or_break  diff echo || return
	
        echo "1" > $TCTMP/tsort.txt
	echo "2" >> $TCTMP/tsort.txt
	echo "a" >> $TCTMP/tsort.txt
	echo "b" >> $TCTMP/tsort.txt
	
	tsort $TCTMP/tsort.txt > $TCTMP/tsort1.tst 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

        echo "1" > $TCTMP/tsort.tst
	echo "a" >> $TCTMP/tsort.tst
	echo "2" >> $TCTMP/tsort.tst
	echo "b" >> $TCTMP/tsort.tst
	
	diff $TCTMP/tsort.tst $TCTMP/tsort1.tst >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}
function TC_wc()
{
	# Check if supporting utilities are available
	tc_exec_or_break  echo grep || return
	
        echo "1" > $TCTMP/wc.txt
	echo "2" >> $TCTMP/wc.txt
	echo "3" >> $TCTMP/wc.txt
	echo "a dog" >> $TCTMP/wc.txt
	echo "b cat" >> $TCTMP/wc.txt
	echo "c duck" >> $TCTMP/wc.txt
	
	wc $TCTMP/wc.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	wc $TCTMP/wc.txt | grep 9 >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "wc -l"
        let TST_TOTAL+=1

	wc -l $TCTMP/wc.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	wc -l $TCTMP/wc.txt | grep 6 >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}
function TC_csplit()
{
	TCNAME="csplit with an integer"

	# Check if supporting utilities are available
	tc_exec_or_break  echo || return
	
        echo "1" > $TCTMP/csplit.txt
	echo "2" >> $TCTMP/csplit.txt
	echo "3" >> $TCTMP/csplit.txt
	echo "a dog" >> $TCTMP/csplit.txt
	echo "b cat" >> $TCTMP/csplit.txt
	echo "c duck" >> $TCTMP/csplit.txt
	
	cd $TCTMP >& /dev/null

	csplit  $TCTMP/csplit.txt 2 >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	if [ -s $TCTMP/xx00 -a -s $TCTMP/xx01 ]; then
		tc_pass_or_fail 0  "$summary3" || return
	else
		tc_pass_or_fail 1  "$summary3" || return
	fi

	tc_register "csplit -f /REGEXP/"
        let TST_TOTAL+=1

	csplit -f csplityy $TCTMP/csplit.txt /cat/ >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	[ -s $TCTMP/csplityy00 -a -s $TCTMP/csplityy01 ]
	tc_pass_or_fail $?  "$summary3"
}
function TC_fmt()
{
	# Check if supporting utilities are available
	tc_exec_or_break  echo grep || return
	
	echo "1.  This is a test file for testing the fmt command." \
		> $TCTMP/fmt.txt
	echo "2.  If a test problem exists, then debug debug until you drop." \
		>> $TCTMP/fmt.txt
	
	fmt $TCTMP/fmt.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	fmt $TCTMP/fmt.txt | grep -c test | grep 1 >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "fmt -w"
        let TST_TOTAL+=1

	fmt -w 50 $TCTMP/fmt.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	fmt -w 50 $TCTMP/fmt.txt | grep -c test | grep 2 >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}
function TC_join()
{
	# Check if supporting utilities are available
	tc_exec_or_break  grep || return
	
        echo "1 duck" > $TCTMP/join.txt
	echo "2" >> $TCTMP/join.txt
	echo "3 cat" >> $TCTMP/join.txt

	echo "1 dog" > $TCTMP/join2.txt
	echo "2 cat" >> $TCTMP/join2.txt
	echo "3 duck" >> $TCTMP/join2.txt
	echo "4 duck" >> $TCTMP/join2.txt
	
	join $TCTMP/join.txt $TCTMP/join2.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	join $TCTMP/join.txt $TCTMP/join2.txt | grep -v 4 >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

function TC_od()
{
	# Check if supporting utilities are available
	tc_exec_or_break  echo grep || return
	
	echo "1" > $TCTMP/od.txt
	echo "2" >> $TCTMP/od.txt
	echo "3" >> $TCTMP/od.txt
	
	od $TCTMP/od.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	od $TCTMP/od.txt | grep 0000006 >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "od -tc"
        let TST_TOTAL+=1

	od -tc $TCTMP/od.txt >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	od -tc $TCTMP/od.txt | grep \n >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

function TC_split()
{
	# Check if supporting utilities are available
	tc_exec_or_break  cd echo || return
	
	echo "1" > $TCTMP/split.txt
	echo "2" >> $TCTMP/split.txt
	echo "3" >> $TCTMP/split.txt
	echo "4" >> $TCTMP/split.txt
	echo "5" >> $TCTMP/split.txt
	echo "6" >> $TCTMP/split.txt
	
	cd $TCTMP >& /dev/null
	
	split $TCTMP/split.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	if [ -s $TCTMP/xaa ]; then
		tc_pass_or_fail 0  "$summary3"
	else
		tc_pass_or_fail 1  "$summary3" || return
	fi

	tc_register "split -l"
        let TST_TOTAL+=1

	split -l 2 $TCTMP/split.txt >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	[ -s xaa -a -s xab -a -s xac ]
	tc_pass_or_fail $?  "$summary3"
}

function TC_tail()
{
	# Check if supporting utilities are available
	tc_exec_or_break grep echo || return

        echo "1" > $TCTMP/tail1.txt
	echo "2" >> $TCTMP/tail1.txt
	echo "3" >> $TCTMP/tail1.txt
	echo "4" >> $TCTMP/tail1.txt
	echo "5 xyz123" >> $TCTMP/tail1.txt
	
        echo "1" > $TCTMP/tail.txt
	echo "2" >> $TCTMP/tail.txt
	echo "3" >> $TCTMP/tail.txt
	echo "4" >> $TCTMP/tail.txt
	echo "5" >> $TCTMP/tail.txt
	echo "6" >> $TCTMP/tail.txt
	echo "7" >> $TCTMP/tail.txt
	echo "8" >> $TCTMP/tail.txt
	echo "9" >> $TCTMP/tail.txt
	echo "10 xyz123" >> $TCTMP/tail.txt

	tail $TCTMP/tail.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	tail $TCTMP/tail.txt | grep 10 >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "tail -with 2 files"
        let TST_TOTAL+=1

	tail $TCTMP/tail.txt $TCTMP/tail1.txt >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return
	
	tail $TCTMP/tail1.txt $TCTMP/tail.txt | grep -c xyz123 | grep 2 > /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "tail -n"
        let TST_TOTAL+=1

	tail -n 7 $TCTMP/tail.txt >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	tail -n 7 $TCTMP/tail.txt | grep -v 1 >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

function TC_unexpand()
{
	# Check if supporting utilities are available
	tc_exec_or_break grep cat echo || return
	
        echo "                16" > $TCTMP/unexpand.txt
	if isbusybox ; then	
		unexpand $TCTMP/unexpand.txt >$TCTMP/unexpand.tst 2>>$stderr
		tc_pass_or_fail $?  "$summary2"
	else
		unexpand $TCTMP/unexpand.txt >$TCTMP/unexpand.tst 2>>$stderr
		tc_fail_if_bad $?  "$summary2" || return

		cat -t $TCTMP/unexpand.tst | grep "\^I" >& /dev/null
		tc_pass_or_fail $?  "$summary3"
	fi
}
function TC_cksum()
{
	# Check if supporting utilities are available
	tc_exec_or_break  awk echo || return
	
        echo "1" > $TCTMP/cksum.txt
	echo "2" >> $TCTMP/cksum.txt
	echo "a" >> $TCTMP/cksum.txt
	echo "b" >> $TCTMP/cksum.txt

	local mychsum=0
	
	cksum $TCTMP/cksum.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return
	
	mychsum=`cksum $TCTMP/cksum.txt | awk '{print $1}' 2> /dev/null`
	if [ $mychsum -ne 1383642305 ]; then
		tc_pass_or_fail 1  "$summary3"

	else
		tc_pass_or_fail 0  "$summary3" 
	fi
}

function TC_cut()
{
	TCNAME="cut -fd"

	# Check if supporting utilities are available
	tc_exec_or_break  echo grep || return
	
        echo "1:two:three" > $TCTMP/cut.txt
	echo "2:3:four" >> $TCTMP/cut.txt
	echo "3 is my test flag zzyyww" >> $TCTMP/cut.txt
	
	cut -f2 -d: $TCTMP/cut.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	cut -f2 -d: $TCTMP/cut.txt | grep two >& /dev/null
	tc_fail_if_bad $?  "$summary2" || return

	cut -f2 -d: $TCTMP/cut.txt | grep zzyyww >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

function TC_fold()
{
	TCNAME="fold -w"

	# Check if supporting utilities are available
	tc_exec_or_break  echo || return
	
        echo "one is that.              one is this." > $TCTMP/fold.txt
        echo "two is this.              two is that." >> $TCTMP/fold.txt
	
	fold -w 20 $TCTMP/fold.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	fold -w 20 $TCTMP/fold.txt | grep -c one | grep 2 >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

function TC_md5sum()
{
	# Check if supporting utilities are available
	tc_exec_or_break  echo grep || return
	
        echo "1" > $TCTMP/md5sum.txt
	echo "2" >> $TCTMP/md5sum.txt
	echo "a" >> $TCTMP/md5sum.txt
	echo "b" >> $TCTMP/md5sum.txt
	
	md5sum  md5sum.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	md5sum  md5sum.txt | grep d4a59fc154c4bba3dd6aa3f5a81de972 >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "md5sum -c"
        let TST_TOTAL+=1

	md5sum  md5sum.txt >$TCTMP/md5sum.tst 2>$stderr

	md5sum -c  md5sum.tst > /dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	md5sum -c  md5sum.tst | grep OK >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

function TC_paste()
{
	# Check if supporting utilities are available
	tc_exec_or_break  echo grep || return
	
        echo "1 This is a test for the paste command for fun for fun for fun." \
	> $TCTMP/paste.txt
	echo "2 That is a test for the paste command for fun for fun for fun." \
	>> $TCTMP/paste.txt
        echo "3 This is a test for the paste command for fun for fun for fun." \
	> $TCTMP/paste.tst
	
	paste $TCTMP/paste.txt $TCTMP/paste.tst >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	paste $TCTMP/paste.txt $TCTMP/paste.tst | grep -c This | grep 1 \
	>& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "paste -s"
        let TST_TOTAL+=1

	paste -s $TCTMP/paste.txt $TCTMP/paste.tst >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	paste -s $TCTMP/paste.txt $TCTMP/paste.tst | grep -c This | grep 2 \
	>& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

function TC_sha1sum()
{
	# Check if supporting utilities are available
	tc_exec_or_break  echo grep || return
	
        echo "1" > $TCTMP/sha1sum.txt
	echo "2" >> $TCTMP/sha1sum.txt
	echo "a" >> $TCTMP/sha1sum.txt
	echo "b" >> $TCTMP/sha1sum.txt
	
	sha1sum  sha1sum.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	sha1sum  sha1sum.txt | grep fda99526e6a2267c6941d424866aaa29d6104b00 \
	>& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "sha1sum -c"
        let TST_TOTAL+=1

	sha1sum  sha1sum.txt >$TCTMP/sha1sum.tst 2>$stderr

	sha1sum -c  sha1sum.tst > /dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	sha1sum -c  sha1sum.tst | grep OK >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

function TC_sum()
{
	# Check if supporting utilities are available
	tc_exec_or_break  echo grep || return
	
        echo "1" > $TCTMP/sum.txt
	echo "2" >> $TCTMP/sum.txt
	echo "a" >> $TCTMP/sum.txt
	echo "b" >> $TCTMP/sum.txt
	
	sum  sum.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	sum  sum.txt | grep 23116 >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

function TC_tr()
{
	# Check if supporting utilities are available
	tc_exec_or_break  echo grep || return
	
        echo "1 his" > $TCTMP/tr.txt
	echo "2 testing" >> $TCTMP/tr.txt
	echo "3 file" >> $TCTMP/tr.txt
	
	tr his her < $TCTMP/tr.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	tr his her < $TCTMP/tr.txt | grep her >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "tr -d"
        let TST_TOTAL+=1

	tr -d his < $TCTMP/tr.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	tr -d his < $TCTMP/tr.txt | grep -v his >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register 'tr -[:..:]'
        let TST_TOTAL+=1

	tr [:lower:] [:upper:] < $TCTMP/tr.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	tr [:lower:] [:upper:] < $TCTMP/tr.txt | grep HIS >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

function TC_uniq()
{
	# Check if supporting utilities are available
	tc_exec_or_break  echo grep || return
	
        echo "a aple" > $TCTMP/uniq.txt
	echo "b banana" >> $TCTMP/uniq.txt
	echo "b banana" >> $TCTMP/uniq.txt
	echo "c cat" >> $TCTMP/uniq.txt
	echo "c cat" >> $TCTMP/uniq.txt
	echo "c cat" >> $TCTMP/uniq.txt
	
	uniq $TCTMP/uniq.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	uniq $TCTMP/uniq.txt | grep -c cat | grep 1  >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "uniq -u"
        let TST_TOTAL+=1

	uniq -u $TCTMP/uniq.txt >/dev/null 2>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	uniq -u $TCTMP/uniq.txt | grep -v cat  >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "uniq -c"
        let TST_TOTAL+=1

	uniq -c $TCTMP/uniq.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	uniq -c $TCTMP/uniq.txt | grep 3  >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

function TC_ptx()
{
	# Check if supporting utilities are available
	tc_exec_or_break  echo || return
	
        echo "1" > $TCTMP/ptx.txt
	echo "2" >> $TCTMP/ptx.txt
	echo "a" >> $TCTMP/ptx.txt
	echo "b" >> $TCTMP/ptx.txt
	
	ptx $TCTMP/ptx.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	ptx $TCTMP/ptx.txt | grep -c a | grep 2 >& /dev/null
	tc_pass_or_fail $?  "$summary3" || return

	tc_register "ptx -O"
        let TST_TOTAL+=1

	ptx -O $TCTMP/ptx.txt >/dev/null 2>>$stderr
	tc_fail_if_bad $?  "$summary2" || return

	ptx -O $TCTMP/ptx.txt | grep "\"b\"" >& /dev/null
	tc_pass_or_fail $?  "$summary3"
}

################################################################################
# main
################################################################################
tc_setup

FRC=0
for cmd in $commands
do
	tc_register $cmd
	TC_$cmd || FRC=$?
done
exit $FRC
