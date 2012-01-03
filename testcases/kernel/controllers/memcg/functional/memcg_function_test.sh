#! /bin/sh

################################################################################
##                                                                            ##
## Copyright (c) 2009 FUJITSU LIMITED                                         ##
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
## Author: Li Zefan <lizf@cn.fujitsu.com>                                     ##
## Restructure for LTP: Shi Weihua <shiwh@cn.fujitsu.com>                     ##
## Added memcg enable/disable functinality: Rishikesh K Rajak		      ##
##						<risrajak@linux.vnet.ibm.com  ##
##                                                                            ##
################################################################################

cd $LTPROOT/testcases/bin
export TCID="memcg_function_test"
export TST_TOTAL=38
export TST_COUNT=0

if [ "x$(grep -w memory /proc/cgroups | cut -f4)" != "x1" ]; then
        echo "WARNING:";
        echo "Either Kernel does not support for memory resource controller or feature not enabled";
        echo "Skipping all memcgroup testcases....";
        exit 0
fi


TEST_PATH=$PWD

PASS=0
FAIL=1

PAGESIZE=`./memcg_getpagesize`
HUGEPAGESIZE=`grep Hugepagesize /proc/meminfo | awk '{ print $2 }'`
[ -z $HUGEPAGESIZE ] && HUGEPAGESIZE=0
HUGEPAGESIZE=$(( $HUGEPAGESIZE * 1024 ))

cur_id=0
failed=0

# Record the test result of a test case
#
# $1 - The result of the test case, $PASS or $FAIL
# $2 - The output information
result()
{
	pass=$1
	info="$2"

	if [ $pass -eq $PASS ]; then
		tst_resm TPASS "$info"
	else
		tst_resm TFAIL "$info"
		failed=$(( $failed + 1 ))
	fi
}

# Check rss size and cache size from memory.stat
#
# $1 - Expected rss size
check_mem_stat()
{
	case $cur_id in
	"11"|"12"|"13")
		# result() will be called in test_failcnt(),not here
		return
		;;
	*)
		;;
	esac

	rss=`cat memory.stat | grep rss | head -n 1 | cut -d " " -f 2`

	if [ "$1" = "$rss" ]; then
		pass=$PASS
	else
		pass=$FAIL
	fi

	result $pass "rss=$rss/$1"
}

warmup()
{
	pid=$1

	case $cur_id in
	"11"|"12"|"13")
		#no warmp here, these are expected to fail
		;;
	*)
		echo "Warming up for test: $cur_id, pid: $pid"
		/bin/kill -s SIGUSR1 $pid 2> /dev/null
		sleep 1
		/bin/kill -s SIGUSR1 $pid 2> /dev/null
		sleep 1

		kill -0 $pid
		if [ $? -ne 0 ]; then
			result $FAIL "cur_id=$cur_id"
			return 1
		else
			echo "Process is still here after warm up: $pid"
		fi
		;;
	esac
	return 0
}


# Run test cases which checks memory.stat after make
# some memory allocation
#
# $1 - the parameters of 'process', such as --shm
# $2 - the -s parameter of 'process', such as 4096
# $3 - the expected rss size
# $4 - check after free ?
test_mem_stat()
{
	echo "Running $TEST_PATH/memcg_process $1 -s $2"
	$TEST_PATH/memcg_process $1 -s $2 &
	sleep 1

	warmup $!
	if [ $? -ne 0 ]; then
		return
	fi

	echo $! > tasks
	/bin/kill -s SIGUSR1 $! 2> /dev/null
	sleep 1

	check_mem_stat $3

	/bin/kill -s SIGUSR1 $! 2> /dev/null
	sleep 1
	if [ $4 -eq 1 ]; then
		check_mem_stat 0
	fi
	/bin/kill -s SIGINT $! 2> /dev/null
}

# Test if memory.failcnt > 0, which means page reclamation
# occured
test_failcnt()
{
	failcnt=`cat memory.failcnt`
	if [ $failcnt -gt 0 ]; then
		pass=$PASS
	else
		pass=$FAIL
	fi

	result $pass "failcnt=$failcnt"
}

# Test process will be killed due to exceed memory limit
#
# $1 - the value of memory.limit_in_bytes
# $2 - the parameters of 'process', such as --shm
# $3 - the -s parameter of 'process', such as 4096
test_proc_kill()
{
	echo $1 > memory.limit_in_bytes
	$TEST_PATH/memcg_process $2 -s $3 &
	pid=$!
	sleep 1
	echo $pid > tasks

	/bin/kill -s SIGUSR1 $pid 2> /dev/null
	sleep 1
	ps -p $pid > /dev/null 2> /dev/null
	if [ $? -ne 0 ]; then
		wait $pid
		if [ $? -eq 1 ]; then
			result $FAIL "process $pid is killed by error"
		else
			result $PASS "process $pid is killed"
		fi
	else
		/bin/kill -s SIGINT $pid 2> /dev/null
		result $FAIL "process $pid is not killed"
	fi
}

# Test memory.limit_in_bytes will be aligned to PAGESIZE
#
# $1 - user input value
# $2 - expected value
test_limit_in_bytes()
{
	echo $1 > memory.limit_in_bytes
	limit=`cat memory.limit_in_bytes`
	if [ $limit -eq $2 ]; then
		result $PASS "input=$1, limit_in_bytes=$limit"
	else
		result $FAIL "input=$1, limit_in_bytes=$limit"
	fi
}

# Test memory controller doesn't charge hugepage
#
# $1 - the value of /proc/sys/vm/nr_hugepages
# $2 - the parameters of 'process', --mmap-file or --shm
# $3 - the -s parameter of 'process', such as $HUGEPAGESIZE
# $4 - 0: expected failure, 1: expected success
test_hugepage()
{
	TMP_FILE=$TEST_PATH/tmp
	nr_hugepages=`cat /proc/sys/vm/nr_hugepages`

	mkdir /hugetlb
	mount -t hugetlbfs none /hugetlb

	echo $1 > /proc/sys/vm/nr_hugepages

	$TEST_PATH/memcg_process $2 --hugepage -s $3 > $TMP_FILE 2>&1 &
	sleep 1

	/bin/kill -s SIGUSR1 $! 2> /dev/null
	sleep 1

	check_mem_stat 0

	echo "TMP_FILE:"
	cat $TMP_FILE

	if [ $4 -eq 0 ]; then
		test -s $TMP_FILE
		if [ $? -eq 0 ]; then
			result $PASS "allocate hugepage failed as expected"
		else
			/bin/kill -s SIGUSR1 $! 2> /dev/null
			/bin/kill -s SIGINT $! 2> /dev/null
			result $FAIL "allocate hugepage shoud fail"
		fi
	else
		test ! -s $TMP_FILE
		if [ $? -eq 0 ]; then
			/bin/kill -s SIGUSR1 $! 2> /dev/null
			/bin/kill -s SIGINT $! 2> /dev/null
			result $PASS "allocate hugepage succeeded"
		else
			result $FAIL "allocate hugepage failed"
		fi
	fi

	sleep 1
	rm -rf $TMP_FILE
	umount /hugetlb
	rmdir /hugetlb
	echo $nr_hugepages > /proc/sys/vm/nr_hugepages
}

# Test the memory charge won't move to subgroup
#
# $1 - memory.limit_in_bytes in parent group
# $2 - memory.limit_in_bytes in sub group
test_subgroup()
{
	mkdir subgroup
	echo $1 > memory.limit_in_bytes
	echo $2 > subgroup/memory.limit_in_bytes

	echo "Running $TEST_PATH/memcg_process --mmap-anon -s $PAGESIZE"
	$TEST_PATH/memcg_process --mmap-anon -s $PAGESIZE &
	sleep 1

	warmup $!
	if [ $? -ne 0 ]; then
		return
	fi

	echo $! > tasks
	/bin/kill -s SIGUSR1 $! 2> /dev/null
	sleep 1
	check_mem_stat $PAGESIZE

	cd subgroup
	echo $! > tasks
	check_mem_stat 0

	# cleanup
	cd ..
	echo $! > tasks
	/bin/kill -s SIGINT $! 2> /dev/null
	sleep 1
	rmdir subgroup
}

# Case 1 - 10: Test the management and counting of memory
testcase_1()
{
	test_mem_stat "--mmap-anon" $PAGESIZE $PAGESIZE 0
}

testcase_2()
{
	test_mem_stat "--mmap-file" $PAGESIZE 0 0
}

testcase_3()
{
	test_mem_stat "--shm -k 3" $PAGESIZE 0 0
}

testcase_4()
{
	test_mem_stat "--mmap-anon --mmap-file --shm" $PAGESIZE \
		      $PAGESIZE 0
}

testcase_5()
{
	test_mem_stat "--mmap-lock1" $PAGESIZE $PAGESIZE 0
}

testcase_6()
{
	test_mem_stat "--mmap-anon" $PAGESIZE $PAGESIZE 1
}

testcase_7()
{
	test_mem_stat "--mmap-file" $PAGESIZE 0 1
}

testcase_8()
{
	test_mem_stat "--shm -k 8" $PAGESIZE 0 1
}

testcase_9()
{
	test_mem_stat "--mmap-anon --mmap-file --shm" $PAGESIZE \
		      $PAGESIZE 1
}

testcase_10()
{
	test_mem_stat "--mmap-lock1" $PAGESIZE $PAGESIZE 1
}

# Case 11 - 13: Test memory.failcnt
testcase_11()
{
	echo $PAGESIZE > memory.limit_in_bytes
	test_mem_stat "--mmap-anon" $(($PAGESIZE*2)) $PAGESIZE 0
	test_failcnt
}

testcase_12()
{
	echo $PAGESIZE > memory.limit_in_bytes
	test_mem_stat "--mmap-file" $(($PAGESIZE*2)) 0 0
	test_failcnt
}

testcase_13()
{
	echo $PAGESIZE > memory.limit_in_bytes
	test_mem_stat "--shm" $(($PAGESIZE*2)) 0 0
	test_failcnt
}

# Case 14 - 15: Test mmap(locked) + alloc_mem > limit_in_bytes
testcase_14()
{
	test_proc_kill $PAGESIZE "--mmap-lock1" $((PAGESIZE*2))
}

testcase_15()
{
	test_proc_kill $PAGESIZE "--mmap-lock2" $((PAGESIZE*2))
}

# Case 16 - 18: Test swapoff + alloc_mem > limi_in_bytes
testcase_16()
{
	swapoff -a
	test_proc_kill $PAGESIZE "--mmap-anon" $((PAGESIZE*2))
	swapon -a
}

testcase_17()
{
	swapoff -a
	test_proc_kill $PAGESIZE "--mmap-file" $((PAGESIZE*2))
	swapon -a
}

testcase_18()
{
	swapoff -a
	test_proc_kill $PAGESIZE "--shm -k 18" $((PAGESIZE*2))
	swapon -a
}

# Case 19 - 21: Test limit_in_bytes == 0
testcase_19()
{
	test_proc_kill 0 "--mmap-anon" $PAGESIZE
}

testcase_20()
{
	test_proc_kill 0 "--mmap-file" $PAGESIZE
}

testcase_21()
{
	test_proc_kill 0 "--shm -k 21" $PAGESIZE
}

# Case 22 - 24: Test limit_in_bytes will be aligned to PAGESIZE
testcase_22()
{
	test_limit_in_bytes $((PAGESIZE-1)) $PAGESIZE
}

testcase_23()
{
	test_limit_in_bytes $((PAGESIZE+1)) $((PAGESIZE*2))
}

testcase_24()
{
	test_limit_in_bytes 1 $PAGESIZE
}

# Case 25 - 28: Test invaild memory.limit_in_bytes
testcase_25()
{
	echo -1 > memory.limit_in_bytes 2> /dev/null
	ret=$?
	tst_kvercmp 2 6 31
	if [ $? -eq 0 ]; then
		result $(( !($ret != 0) ))  "return value is $ret"
       else
		result $(( !($ret == 0) ))  "return value is $ret"
	fi
}

testcase_26()
{
	echo 1.0 > memory.limit_in_bytes 2> /dev/null
	result $(( !($? != 0) )) "return value is $?"
}

testcase_27()
{
	echo 1xx > memory.limit_in_bytes 2> /dev/null
	result $(( !($? != 0) )) "return value is $?"
}

testcase_28()
{
	echo xx > memory.limit_in_bytes 2> /dev/null
	result $(( !($? != 0) )) "return value is $?"
}

# Case 29 - 35: Test memory.force_empty
testcase_29()
{
	$TEST_PATH/memcg_process --mmap-anon -s $PAGESIZE &
	pid=$!
	sleep 1
	echo $pid > tasks
	/bin/kill -s SIGUSR1 $pid 2> /dev/null
	sleep 1
	echo $pid > ../tasks

	echo 1 > memory.force_empty
	if [ $? -eq 0 ]; then
		result $PASS "force memory succeeded"
	else
		result $FAIL "force memory failed"
	fi

	/bin/kill -s SIGINT $pid 2> /dev/null
}

testcase_30()
{
	$TEST_PATH/memcg_process --mmap-anon -s $PAGESIZE &
	pid=$!
	sleep 1
	echo $pid > tasks
	/bin/kill -s SIGUSR1 $pid 2> /dev/null
	sleep 1

	echo 1 > memory.force_empty 2> /dev/null
	if [ $? -ne 0 ]; then
		result $PASS "force memory failed as expected"
	else
		result $FAIL "force memory should fail"
	fi

	/bin/kill -s SIGINT $pid 2> /dev/null
}

testcase_31()
{
	echo 0 > memory.force_empty 2> /dev/null
	result $? "return value is $?"
}

testcase_32()
{
	echo 1.0 > memory.force_empty 2> /dev/null
	result $? "return value is $?"
}

testcase_33()
{
	echo 1xx > memory.force_empty 2> /dev/null
	result $? "return value is $?"
}

testcase_34()
{
	echo xx > memory.force_empty 2> /dev/null
	result $? "return value is $?"
}

testcase_35()
{
	# writing to non-empty top mem cgroup's force_empty
	# should return failure
	echo 1 > /dev/memcg/memory.force_empty 2> /dev/null
	result $(( !$? )) "return value is $?"
}

# Case 36 - 38: Test that group and subgroup have no relationship
testcase_36()
{
	test_subgroup $PAGESIZE $((2*PAGESIZE))
}

testcase_37()
{
	test_subgroup $PAGESIZE $PAGESIZE
}

testcase_38()
{
	test_subgroup $PAGESIZE 0
}

shmmax=`cat /proc/sys/kernel/shmmax`
if [ $shmmax -lt $HUGEPAGESIZE ]; then
	echo $(($HUGEPAGESIZE)) > /proc/sys/kernel/shmmax
fi

cleanup()
{
	killall -9 memcg_process 2>/dev/null
	if [ -e /dev/memcg ]; then
		umount /dev/memcg 2>/dev/null
		rmdir /dev/memcg 2>/dev/null
	fi
}


do_mount()
{
	cleanup;

	mkdir /dev/memcg 2> /dev/null
	mount -t cgroup -omemory memcg /dev/memcg
}

# Run all the test cases
for i in $(seq 1 $TST_TOTAL)
do
	export TST_COUNT=$(( $TST_COUNT + 1 ))
	cur_id=$i

	do_mount;

	# prepare
	mkdir /dev/memcg/$i 2> /dev/null
	cd /dev/memcg/$i

	# run the case
	testcase_$i

	# clean up
	sleep 1
	cd $TEST_PATH
	rmdir /dev/memcg/$i

	cleanup;
done

echo $shmmax > /proc/sys/kernel/shmmax

if [ $failed -ne 0 ]; then
	exit 1
else
	exit 0
fi
