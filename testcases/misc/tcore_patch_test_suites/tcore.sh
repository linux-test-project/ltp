#!/bin/sh

#    Copyright (c) Guo Min <min.guo@intel.com>., 2002
#
#    This program is free software;  you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY;  without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#    the GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program;  if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

# Test suite for the t-core kernel patch 
export TCID=tcore01
PTHREAD_DIR="/lib/i686"
BIN_DIR=$LTPROOT/testcases/bin/
TEST_DIR=$(mktemp -dt)
OSTYPE="redhat"
# Do some preparation to generate the correct core dump
# files
pass=0
fail=0
check_install()
{
	if [ -f /proc/sys/kernel/core_dumps_threads ];then		
		pass=`expr $pass + 1`
		return 0
	else
		fail=`expr $fail + 1`
		return 1
	fi
}
os_check()
{
	if [ -d /usr/src/hardhat ];then
		OSTYPE="hardhat"
	fi
}

cleanup()
{
		rm -rf $TEST_DIR
		[ -f /proc/sys/kernel/core_dumps_threads ] && ! [ -z "$OLD_PATTERN" ] && 
		echo "$OLD_PATTERN" > /proc/sys/kernel/core_dumps_threads
		exit $1
}

prepare_dump()
{
	ulimit -c 20000000 >/dev/null 2>&1
	# Open the core_dumps_threads key
	if [ -f /proc/sys/kernel/core_dumps_threads ];then
		m=`cat /proc/sys/kernel/core_dumps_threads`
		if [ $m -eq 0 ];then
			echo "1">/proc/sys/kernel/core_dumps_threads
		fi
	fi
	if [ -f /proc/sys/kernel/core_pattern ]; then
			OLD_PATTERN=$(cat /proc/sys/kernel/core_pattern)
			echo 'core.%p' > /proc/sys/kernel/core_pattern
	fi
	#Recover the lipthread.so.0 lib to generate the correct
	#core dump file
	os_check
	if [ $OSTYPE = "redhat" ];then
		cd $PTHREAD_DIR && {
			if [ -f libpthread.so.0.orig ];then
				test=`ls -l libpthread.so.0.orig|awk '{print $11}'`
					if [ $test = "libpthread-0.9.so"  ];then
						mv libpthread.so.0.orig libpthread.so.0
					fi
			fi
			cd -
		}
	fi
}
#Prepare for the gdb dump 
prepare_gdb()
{
	os_check
	if [ $OSTYPE = "redhat" ];then
		cd $PTHREAD_DIR && {
		if [ -f libpthread.so.0 ];then
			test=`ls -l libpthread.so.0|awk '{print $11}'`
			if [ $test = "libpthread-0.9.so"  ];then
				mv libpthread.so.0 libpthread.so.0.orig
			fi
		fi
		ln -s /lib/libpthread.so.0 libpthread.so.0 >/dev/null 2>&1
		cd -
    }
	fi
}
#Test whether the system can generate the needed core files
#The core files should has pid as its extension
Test_gen_core()
{
	cd $TEST_DIR
	prepare_dump
	if [ -f core.* ];then
		rm -f core.*
	fi
	pid=`$BIN_DIR/tcore |grep "consumer pid"|awk '{print $2}'|cut -d = -f 2`
	echo "Test whether we can generate the needed core file"
	if [ -f core.* ];then
		echo "PASS"
		pass=`expr $pass + 1`
		return 0
	else
		echo "FAIL"
		fail=`expr $fail + 1`
		return 1
	fi	
}

Test_core_file()
{
	cd $TEST_DIR
	prepare_gdb
	mv -f $TEST_DIR/core.* $TEST_DIR/corefile >/dev/null 2>&1
	echo "Test whether the core support bt,fpu and threads commands "
	expect $BIN_DIR/tcore.exp
	return=$?
	pass=`expr $pass + $return`
	fail=`expr 3 - $return + $fail`
}

echo "Test Tcore patch "
check_install
Test_gen_core
Test_core_file
echo "Linux Tcore test results" > results
echo "Total pass fail" >>  results
echo "  5	$pass	$fail" >> results
cleanup $fail 

	
	
