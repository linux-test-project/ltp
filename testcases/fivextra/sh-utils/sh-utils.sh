#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab:
################################################################################
##									      ##
## (C) Copyright IBM Corp. 2003						      ##
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
# File :	sh-utils.sh
#
# Description:	Test the functions provided by sh-utils.
#
# Author:	Robert Paulsen, rpaulsen@us.ibm.com
#
# History:	Feb 06 2003 - Created. Robert Paulsen. rpaulsen@us.ibm.com
#		Oct 29 2003 - Re-initialized TST_TOTAL so it does not report
#			      one tc brok at the end.
#		16 Dec 2003 - (robert) updated to tc_utils.source
#		09 Aug 2004 - Modified by Gong Jie <gongjie@cn.ibm.com>
#			      Suit 64-bit dynamically linked bash on PPC-64
#		09 Aug 2004 - (rcp) copied in do_chroot re-write from 
#				busybox version of the test (and was careful
#				to preserved Gong Jie's fix).

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

# commands to be tested
names=" [ basename date echo false pwd sleep stty su true uname chroot \
	dirname env expr factor groups id logname nice nohup pathchk pinky \
	printenv printf seq tee test tty users who whoami yes"

TST_TOTAL=0
for i in $names ; do
	let TST_TOTAL+=1
done

fullfilename=$0

################################################################################
# the testcase functions
################################################################################

function do_basename()
{
	tc_exec_or_break || return
	local actual="`basename $0 2>$stderr`"
	[ "$actual" = "${0##*/}" ]
	tc_pass_or_fail $? "basname expected \"$thisfilename\" but got \"$actual\""
}

function do_date()
{
	tc_exec_or_break sleep || return
	local -i start finis delta
	tc_info "Pausing for 5 seconds..."
	start=`date +%s 2>$stderr`
	sleep 5
	finis=`date +%s 2>$stderr`
	let delta=$finis-$start
	[ $delta -ge 4 ] && [ $delta -le 6 ]
	tc_pass_or_fail $?
}

function do_echo()
{
	tc_exec_or_break grep || return
	echo "hello sailor" 2>$stderr | grep "hello sailor" >/dev/null
	tc_pass_or_fail $?
}

function do_false()
{
	! false 2>$stderr
	tc_pass_or_fail $?
}

function do_pwd()
{
	tc_exec_or_break mkdir grep || return
	mkdir -p $TCTMP/do_pwd
	cd $TCTMP/do_pwd
	local actual1=`pwd 2>$stderr`
	cd -
	local actual2=`pwd 2>>$stderr`
	echo $actual1 | grep "do_pwd" >/dev/null && \
	! echo $actual2 | grep "do_pwd" >/dev/null
	tc_pass_or_fail $?
}

function do_sleep()
{
	tc_exec_or_break date || return
	local -i start finis delta
	tc_info "Pausing for 5 seconds..."
	start=`date +%s`
	sleep 5 2>$stderr
	finis=`date +%s`
	let delta=$finis-$start
	[ $delta -ge 4 ] && [ $delta -le 6 ]
	tc_pass_or_fail $?
}

function do_stty()
{
	tc_exec_or_break grep || return
	stty -a >$stdout 2>$stderr
	tc_fail_if_bad $? "unexpected response" || return
#
	grep -q "speed"		<$stdout && \
	grep -q "baud"		<$stdout && \
	grep -q "rows"		<$stdout && \
	grep -q "columns"	<$stdout && \
	grep -q "line"		<$stdout && \
	grep -q "intr ="	<$stdout && \
	grep -q "quit ="	<$stdout && \
	grep -q "erase ="	<$stdout && \
	grep -q "kill ="	<$stdout && \
	grep -q "eof ="		<$stdout && \
	grep -q "eol ="		<$stdout && \
	grep -q "eol2 ="	<$stdout && \
	grep -q "start ="	<$stdout && \
	grep -q "stop ="	<$stdout && \
	grep -q "susp ="	<$stdout && \
	grep -q "rprnt ="	<$stdout && \
	grep -q "werase ="	<$stdout && \
	grep -q "lnext ="	<$stdout && \
	grep -q "flush ="	<$stdout && \
	grep -q "min ="		<$stdout && \
	grep -q "time ="	<$stdout 
	tc_pass_or_fail $?
}

function do_su()
{
	tc_root_or_break || return
	tc_exec_or_break cat pwd sed || return
	echo -n "" > $stderr

	# create a temp user
	tc_add_user_or_break || return

	# check that "su temp_user" switches to user
	echo "echo \$USER" | su $temp_user &>$TCTMP/output
	local user_name=`cat $TCTMP/output`
	[ "$user_name" = "$temp_user" ]
	tc_fail_if_bad $? "expected userid \"$temp_user\", got \"$user_name\"" || return

	# check that "su temp_user" stays in original pwd
	echo -n "" > $stderr
	echo "echo `pwd`" | su $temp_user &>$TCTMP/output
	local temp_pwd=`cat $TCTMP/output`
	local real_pwd="`pwd`"
	[ "$temp_pwd" = "$real_pwd" ]
	tc_fail_if_bad $? "expected directory \"$real_pwd\", got \"$temp_pwd\"" || return

	# check that "su - temp_user" switches to user
	echo -n "" > $stderr
	echo "echo \$USER" | su - $temp_user &>$TCTMP/output
	local user_name=`cat $TCTMP/output`
	[ "$user_name" = "$temp_user" ]
	tc_fail_if_bad $? "expected userid \"$temp_user\", got \"$user_name\"" || return

	# check that "su - temp_user" sets new pwd
	# This causes temp_user to create a file in $HOME and immediately
	# exit back to root. Root then checks that the file tc_exist_or_break and has
	# the correct contents. If so, "su - temp_user" is deemed to have
	# succeeded.
	echo -n "" > $stderr
	echo "echo hello > hello.txt" | su - $temp_user 2>>$stderr
	local home=`sed -n "/^\$temp_user:/p" /etc/passwd | cut -d':' -f 6`
	local actual="`cat $home/hello.txt`"
	[ "$actual" = "hello" ]
	tc_pass_or_fail $? "expected \"hello\", got \"$actual\""
}

function do_true()
{
	true 2>$stderr
	tc_pass_or_fail $? "\"true\" did not return \"0\""
}

function do_uname()
{
	tc_exec_or_break cat || return
	tc_exist_or_break "/proc/sys" || return
	local ostype=`cat /proc/sys/kernel/ostype`
	local hostname=`cat /proc/sys/kernel/hostname`
	local osrelease=`cat /proc/sys/kernel/osrelease`
	local version=`cat /proc/sys/kernel/version`
	local expected="$ostype $hostname $osrelease $version"
	local actual=`uname -a 2>$stderr`
	echo $actual | grep "$expected" >/dev/null
	tc_pass_or_fail $? "expected=$expected, actual=$actual"
}

function do_LBR()	# left square bracket
{
	tc_exec_or_break grep || return
	[ 2>&1 | grep "\[: missing" >/dev/null
	tc_pass_or_fail $?
}

function do_chroot_shell_no_ldd()
{
	local SHELL=$1

	if tc_is_busybox $SHELL ; then
		cp `which busybox` $TCTMP/fakeroot/bin
		ln -s $TCTMP/fakeroot/bin/busybox $TCTMP/fakeroot/$SHELL
	else
		cp $SHELL $TCTMP/fakeroot/$SHELL
	fi

	cp /lib/libreadline.so.* $TCTMP/fakeroot/lib
	cp /lib/libhistory.so.* $TCTMP/fakeroot/lib
	cp /lib/libncurses.so.* $TCTMP/fakeroot/lib
	cp /lib/libdl.so.* $TCTMP/fakeroot/lib
	cp /lib/libc.so.* $TCTMP/fakeroot/lib
	cp /lib/ld*.so.* $TCTMP/fakeroot/lib

	chroot $TCTMP/fakeroot $SHELL /doit >$stdout 2>$stderr
	cat $TCTMP/fakeroot/hello.txt | grep "Hello Sailor" >/dev/null
	if [ $? -ne 0 ] ; then
		tc_info "Since there is no ldd on this system,"
		tc_info "this testcase used a hard-coded list"
		tc_info "of libraries that $SHELL is expectd"
		tc_info "to require. If this list is wrong,"
		tc_info "it may be the cause of this failure."
		tc_info "Please investigate and see if the"
		tc_info "hard-coded list needs to change."
		tc_pass_or_fail 1 "did not execute script in chrooted jail"
	else
		tc_pass_or_fail 0
	fi
}

function do_chroot_shell_ldd()
{
	SHELL=$1

	if tc_is_busybox $SHELL ; then
		cp $(which busybox) $TCTMP/fakeroot/bin
		ln -s $TCTMP/fakeroot/bin/busybox $TCTMP/fakeroot/$SHELL
	else
		cp $SHELL $TCTMP/fakeroot/$SHELL
	fi

	ldd $SHELL > $TCTMP/libs
	while read junk junk lib junk ; do
		cp $lib $TCTMP/fakeroot/lib
	done < $TCTMP/libs
	chroot $TCTMP/fakeroot $SHELL /doit >$stdout 2>$stderr
	tc_fail_if_bad $? "chroot failed"
	cat $TCTMP/fakeroot/hello.txt 2>$stderr | grep "Hello Sailor" >$stdout 2>>$stderr
	tc_pass_or_fail $? "did not execute script in chrooted jail"
}

function do_chroot_no_shell()
{
	tc_info "Without ash or sh this test simply"
	tc_info "checks that chroot returns the proper"
	tc_info "error message."
	local result=`chroot $TCTMP xyz 2>&1`
	local exp
	exp="chroot: cannot execute xyz: No such file or directory"
	[ "$result" = "$exp" ]
	tc_pass_or_fail $? "bad result: \"$result\""
}

function do_chroot() {
	tc_exec_or_break chmod mkdir cat || return
	mkdir -p $TCTMP/fakeroot/bin
	mkdir -p $TCTMP/fakeroot/lib
	ln -s lib $TCTMP/fakeroot/lib64
	cat > $TCTMP/fakeroot/doit <<-EOF
		#!$SHELL
		# something to run in fakeroot
		echo "Hello Sailor" > hello.txt
	EOF
	chmod +x $TCTMP/fakeroot/doit
	cp $(which echo) $TCTMP/fakeroot/bin

	local ldd_path=$(which ldd 2>/dev/null)
	local bash_path=$(which bash 2>/dev/null)
	local ash_path=$(which ash 2>/dev/null)
	local static_ash_path=$(which ash.static 2>/dev/null)

	if [ "$ldd_path" ] ; then
		if [ "$bash_path" ] ; then
			if ldd $bash_path &>/dev/null ; then
				do_chroot_shell_ldd $bash_path
				return
			fi
		fi
		if [ "$ash_path" ] ; then
			if ldd $ash_path &>/dev/null ; then
				do_chroot_shell_ldd $ash_path
				return
			fi
		fi
		return 0
	fi
	if [ "$bash_path" ] ; then
		do_chroot_shell_no_ldd $bash_path
		return
	elif [ "$ash_path" ] ; then
		do_chroot_shell_no_ldd $ash_path
		return
	elif [ "$static_ash_path" ] ; then
		do_chrot_shell_no_ldd $static_ash_path
		return
	fi
	do_chroot_no_shell
}

function do_dirname()
{
	tc_exec_or_break touch mkdir || return
	mkdir -p $TCTMP/dir
	touch $TCTMP/dir/file
	local dirname=`dirname $TCTMP/dir/file 2>$stderr`
	[ "$TCTMP/dir" = "$dirname" ]
	tc_pass_or_fail $?
}

function do_env()
{
	tc_exec_or_break grep || return
	export XXX="hello sailor"
	env 2>$stderr | grep "XXX" | grep "hello sailor" > /dev/null
	tc_pass_or_fail $?
}

function do_expr()
{
	tc_exec_or_break || return
	local val1=`expr 7 \* 6 2>>$stderr`
	local exp1=42
	local val2=`expr length "hello sailor" 2>>$stderr`
	local exp2=12
	local val3=`expr "abc" : "a\(.\)c" 2>$stderr`
	local exp3=b
	local failure
	[ "$val1" != "$exp1" ] && \
		failure="expected $exp1, got $val1"
	[ -z "$failure" ] && [ "$val2" != "$exp2" ] && \
		failure="expected $exp2, got $val2"
	[ -z "$failure" ] && [ "$val3" != "$exp3" ] && \
		failure="expected $exp3, got $val3"
	[ -z "$failure" ]
	tc_pass_or_fail $? "$failure"
}

function do_factor()
{
	tc_exec_or_break || return
	local exp="111: 3 37"
	local act=`factor 111 2>$stderr`
	local failure
	[ "$exp" = "$act" ] || failure="expected $exp, got $act"
	[ -z "$failure" ]
	tc_pass_or_fail $? "$failure"
}

function do_groups()
{
	tc_exec_or_break grep cat sort uniq || return
#
	# put expected groups in file 
	cat /etc/group | grep $USER | cut -f1 -d: > $TCTMP/groups
	local defgid="`cat /etc/passwd | grep \"^$USER\" | cut -f4 -d:`"
	cat /etc/group | grep ":$defgid:" | cut -f1 -d: >> $TCTMP/groups
	cat $TCTMP/groups | sort | uniq > $TCTMP/expected
	local expected="`cat $TCTMP/expected`"
#
	# get results from groups command
	local temp="`groups 2>$stderr`"
	local g
	local -i i=0
	cat /dev/null > $TCTMP/groups2
	for g in $temp ; do
		#let ++i
		#[ $i -lt 3 ] && continue	# skip uid and colon
		echo $g >> $TCTMP/groups2	# multiline for sorting
	done
	local results=`cat $TCTMP/groups2 | sort`
	[ "$expected" = "$results" ]
	tc_pass_or_fail $? "expected \"$expected\", got \"$results\""
}

function do_id()
{
	tc_exec_or_break echo || return
	local actual=`id $USER 2>$stderr`
	tc_fail_if_bad $? "unexpected results" "rc=$?"
#
	local actual1=`echo $actual | grep "($USER)"`
	local actual2=`echo $actual | grep uid=`
	local actual3=`echo $actual | grep gid=`
	local actual4=`echo $actual | grep groups=`
	[ "$actual1" ] && \
	[ "$actual2" ] && \
	[ "$actual3" ]
	tc_pass_or_fail $? \
		"expected \"($USER)\", \"uid=\", \"gid=\", got \"$actual\"" 
}

function do_logname()
{
	tc_exec_or_break wc echo || return
	local actual=`logname 2>&1`
	local words
	declare -i words=`echo $actual | wc -w`
	[ $words -eq 1 ] || [ "$actual" = "logname: no login name" ]
	tc_pass_or_fail $? "expected a username or \"no login name\", got \"$actual\""
}

function do_nice()
{
	tc_exec_or_break cat echo chmod ps grep || return
	tc_info "ugly \"Terminated\" message is normal."
	cat > $TCTMP/niceme$$ <<-EOF
		#!$SHELL
		# infinite loop script to be niced
		while : ; do
			:
		done
	EOF
	chmod +x $TCTMP/niceme$$
	nice -n +10 $TCTMP/niceme$$ & &>/dev/null
	killme=$!
	local pid nice cmd
	read pid nice cmd <<-EOF
		`ps --format "pid nice cmd" | grep [n]iceme$$`
	EOF
	kill $killme &>/dev/null
	[ "$nice" -eq "10" ] && [ "$pid" -eq "$killme" ]
	tc_pass_or_fail $? "expected nice=\"10\" and pid=\"$killme\", got nice=\"$nice\" and pid=\"$pid\""
}

function do_nohup()
{
	# NOTE: the "sleep" command doesn't die even without the nohup.
	#	This is because it doesn't believe it is running in a login
	#	shell, which is odd because the "su -l" is supposed to start
	#	a login shell. Therefore I am at a loss as how to automate
	#	this test.
	tc_root_or_break || return
	tc_exec_or_break kill echo sleep ps su || return
	tc_add_user_or_break || return
	tc_info "The nohup test really only checks"
	tc_info "that nohup doesn't crash and burn."
	tc_info "Functionally it does little."
	echo "nohup sleep 10000 >/dev/null &" | su -l $temp_user
	sleep 1	# wait for the above to happen (it's asynchronous)
	local line=`ps -ef | grep "[s]leep 10000" `
	local pid=`echo $line | cut -f2 -d" "`
	[ "$line" ]
	tc_pass_or_fail $? "Process died but shouldn't have"
#
	[ "$pid" ] && kill $pid	# kill the no_hupped process
	tc_del_user_or_break
}

function do_pathchk()
{
	tc_root_or_break || return
	tc_exec_or_break su mkdir chmod || return
	tc_add_user_or_break || return
	mkdir $TCTMP/pathchk
	touch $TCTMP/pathchk/xxx
	chmod a-x $TCTMP/pathchk
	local result=`echo "pathchk $TCTMP/pathchk/xxx 2>&1" | \
		su $temp_user`
	echo $result | grep "$TCTMP/pathchk" >/dev/null &&
	echo $result | grep "is not searchable" >/dev/null
	tc_pass_or_fail $? "$result"
	tc_del_user_or_break
}

function do_pinky()
{
	tc_exec_or_break grep || return
	pinky  2>$stderr | grep "Login" >/dev/null
	tc_pass_or_fail $?
}

function do_printenv()
{
	tc_exec_or_break grep || return
	export local XXX=Hello
	export local YYY=Sailor
	local myenv="`printenv XXX YYY`"
	echo $myenv | grep Hello &>/dev/null && \
	echo $myenv | grep Sailor &>/dev/null
	tc_pass_or_fail $? "expected \"$XXX$ $YYY\", got $myenv"
}

function do_printf()
{
	local result=`printf "hello %s. float=%g\n" "sailor" "3.14159"`
	local expected="hello sailor. float=3.14159"
	[ "$result" = "$expected" ]
	tc_pass_or_fail $? "expected \"$expected\", got \"$result\""
}

function do_seq()
{
	local arg1="-s: -f%g 3 7 50"
	local arg2="-s: -f%f 3 7 50"
	local arg3="-s: -f%e 3 7 50"
	local exp1="3:10:17:24:31:38:45"
	local exp2="3.000000:10.000000:17.000000:24.000000:31.000000:38.000000:45.000000"
	local exp3="3.000000e+00:1.000000e+01:1.700000e+01:2.400000e+01:3.100000e+01:3.800000e+01:4.500000e+01"
	local act1=""
	local act2=""
	local act3=""
	local failed=""
	for x in 1 2 3 ; do
		eval act$x=$(eval seq \$arg$x)
		if ! eval [ "\$exp$x" = "\$act$x" ] ; then
			eval local expected="\$exp$x"
			eval local actual="\$act$x"
			local failed="$failed expected $expected, got $actual"
			break
		fi
	done
	[ -z "$failed" ]
	tc_pass_or_fail $? "$failed"
}

function do_tee()
{
	tc_exec_or_break echo cat || return
	local expected="Hello Sailor"
	echo "$expected" | tee $TCTMP/file1 $TCTMP/file2 \
		> $TCTMP/file3 2>$stderr
	local file1=`cat $TCTMP/file1`
	local file2=`cat $TCTMP/file2`
	local file3=`cat $TCTMP/file3`
	[ "$file1" = "$expected" ] && \
	[ "$file2" = "$expected" ] && \
	[ "$file3" = "$expected" ]
	tc_pass_or_fail $? \
		"\"$file1\", \"$file2\", \"$file3\" should equal \"$expected\""
}

function do_test
{
	tc_exec_or_break touch ln || return
	local tmp1=$TCTMP/xxx
	local tmp2=$TCTMP/yyy
	local cmd1="test -f $fullfilename"
	local cmd2="test -x $fullfilename"
	local cmd3="test ! -z $fullfilename"
	local cmd4="test -L $tmp2"
	touch $tmp1
	ln -s $tmp1 $tmp2
	local failed=""
	for x in 1 2 3 4 ; do	# count must match the above cmd list
		eval \$cmd$x 2>$stderr || eval failed="\" \$cmd$x failed\""
		[ "$failed" ] && break
	done
	[ -z "$failed" ]
	tc_pass_or_fail $? "$failed"
}

function do_tty()
{
	tc_exec_or_break grep || return
	tty 2>$stderr | grep "/dev" &>/dev/null || \
	tty 2>$stderr | grep "not a tty" &>/dev/null
	tc_pass_or_fail $?
}

function do_users()
{
	tc_exec_or_break grep cut || return
	local name=`users 2>$stderr | cut -f1 -d" "`
	grep "$name" /etc/passwd &>/dev/null
	tc_pass_or_fail $? "users returned \"$name\" which is not a valid user"
}

function do_who()
{
	tc_exec_or_break grep cut || return
	local name=`who 2>$stderr | cut -f1 -d" "`
	grep "$name" /etc/passwd &>/dev/null
	tc_pass_or_fail $? "who returned \"$name\" which is not a valid user"
}

function do_whoami()
{
	tc_root_or_break || return
	tc_exec_or_break echo su cat || return
	tc_add_user_or_break || return
	echo "whoami" | su $temp_user > $TCTMP/user
	local actual="`cat $TCTMP/user`"
	[ "$actual" = "$temp_user" ]
	tc_pass_or_fail $? "expected \"$temp_user\", got \"$actual\""
	tc_del_user_or_break
}

function do_yes()
{
	tc_exec_or_break echo || return
	echo > $TCTMP/read <<-EOF
		#!$SHELL
		read a; read b; read c; read d; read e
		[ "$a" = "sailor" ] && \
		[ "$b" = "sailor" ] && \
		[ "$c" = "sailor" ] && \
		[ "$d" = "sailor" ] && \
		[ "$e" = "sailor" ] && exit 0
		exit 1
	EOF
	yes "sailor" | source $TCTMP/read
	tc_pass_or_fail $?
}

function do_unknown() {
	tc_pass_or_fail 0 "unknown command: \"$TCNAME\""
}

################################################################################
# main
################################################################################

tc_setup

#
# run tests against all commands in sh-utils package.
#
for name in $names ; do
	tc_register $name
	case $name in
		[)		do_LBR		; ;;
		basename)	do_basename	; ;;
		chroot)		do_chroot	; ;;
		date)		do_date		; ;;
		dirname)	do_dirname	; ;;
		echo)		do_echo		; ;;
		env)		do_env		; ;;
		expr)		do_expr		; ;;
		factor)		do_factor	; ;;
		false)		do_false	; ;;
		groups)		do_groups	; ;;
		id)		do_id		; ;;
		logname)	do_logname	; ;;
		nice)		do_nice		; ;;
		nohup)		do_nohup	; ;;
		pathchk)	do_pathchk	; ;;
		pinky)		do_pinky	; ;;
		printenv)	do_printenv	; ;;
		printf)		do_printf	; ;;
		pwd)		do_pwd		; ;;
		seq)		do_seq		; ;;
		sleep)		do_sleep	; ;;
		stty)		do_stty		; ;;
		su)		do_su		; ;;
		tee)		do_tee		; ;;
		test)		do_test		; ;;
		true)		do_true		; ;;
		tty)		do_tty		; ;;
		uname)		do_uname	; ;;
		users)		do_users	; ;;
		who)		do_who		; ;;
		whoami)		do_whoami	; ;;
		yes)		do_yes		; ;;
		*)		do_unknown	; ;;
	esac
done
