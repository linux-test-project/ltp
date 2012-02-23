#! /bin/sh

################################################################################
##                                                                            ##
## Copyright (c) 2012 FUJITSU LIMITED                                         ##
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
## Author: Peng Haitao <penght@cn.fujitsu.com>                                ##
##                                                                            ##
################################################################################

if [ "x$(grep -w memory /proc/cgroups | cut -f4)" != "x1" ]; then
	echo "WARNING:";
	echo "Either Kernel does not support for memory resource controller or feature not enabled";
	echo "Skipping all memcgroup testcases....";
	exit 0
fi

cd $LTPROOT/testcases/bin

TEST_PATH=$PWD
PAGESIZE=`./memcg_getpagesize`
HUGEPAGESIZE=`grep Hugepagesize /proc/meminfo | awk '{ print $2 }'`
[ -z $HUGEPAGESIZE ] && HUGEPAGESIZE=0
HUGEPAGESIZE=$(( $HUGEPAGESIZE * 1024 ))
PASS=0
FAIL=1

cur_id=0
failed=0

# Record the test result of a test case
# $1 - The result of the test case, $PASS or $FAIL
# $2 - The output information
result()
{
	local pass=$1
	local info="$2"

	if [ $pass -eq $PASS ]; then
		tst_resm TPASS "$info"
	else
		tst_resm TFAIL "$info"
		: $(( failed += 1 ))
	fi
}

# Check size in memcg
# $1 - Item name
# $2 - Expected size
check_mem_stat()
{
	if [ -e $1 ]; then
		item_size=`cat $1`
	else
		item_size=`grep -w $1 memory.stat | cut -d " " -f 2`
	fi

	if [ "$2" = "$item_size" ]; then
		pass=$PASS
	else
		pass=$FAIL
	fi

	result $pass "$1=$item_size/$2"
}

warmup()
{
	pid=$1

	echo "Warming up for test: $cur_id, pid: $pid"
	kill -s USR1 $pid 2> /dev/null
	sleep 1
	kill -s USR1 $pid 2> /dev/null
	sleep 1

	kill -0 $pid
	if [ $? -ne 0 ]; then
		result $FAIL "cur_id=$cur_id"
		return 1
	else
		echo "Process is still here after warm up: $pid"
	fi

	return 0
}

# Run test cases which checks memory.stat after make
# some memory allocation
# $1 - the parameters of 'process', such as --shm
# $2 - the -s parameter of 'process', such as 4096
# $3 - item name in memory.stat
# $4 - the expected size
# $5 - check after free ?
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
	kill -s USR1 $! 2> /dev/null
	sleep 1

	check_mem_stat $3 $4

	kill -s USR1 $! 2> /dev/null
	sleep 1
	if [ $5 -eq 1 ]; then
		check_mem_stat $3 0
	fi

	kill -s INT $! 2> /dev/null
}

# Run test cases which checks memory.max_usage_in_bytes after make
# some memory allocation
# $1 - the parameters of 'process', such as --shm
# $2 - the -s parameter of 'process', such as 4096
# $3 - item name
# $4 - the expected size
# $5 - check after free ?
test_max_usage_in_bytes()
{
	echo "Running $TEST_PATH/memcg_process $1 -s $2"
	$TEST_PATH/memcg_process $1 -s $2 &
	sleep 1

	warmup $!
	if [ $? -ne 0 ]; then
		return
	fi

	echo $! > tasks
	kill -s USR1 $! 2> /dev/null
	sleep 1

	kill -s USR1 $! 2> /dev/null
	sleep 1

	check_mem_stat $3 $4

	if [ $5 -eq 1 ]; then
		echo 0 > $3
		check_mem_stat $3 0
	fi

	kill -s INT $! 2> /dev/null
}

# make some memory allocation
# $1 - the parameters of 'process', such as --shm
# $2 - the -s parameter of 'process', such as 4096
malloc_free_memory()
{
	echo "Running $TEST_PATH/memcg_process $1 -s $2"
	$TEST_PATH/memcg_process $1 -s $2 &
	sleep 1

	echo $! > tasks
	kill -s USR1 $! 2> /dev/null
	sleep 1

	kill -s USR1 $! 2> /dev/null
	sleep 1

	kill -s INT $! 2> /dev/null
}

# Test if failcnt > 0, which means page reclamation occured
# $1 - item name in memcg
test_failcnt()
{
	failcnt=`cat $1`
	if [ $failcnt -gt 0 ]; then
		pass=$PASS
	else
		pass=$FAIL
	fi

	result $pass "$1=$failcnt"
}

# Test process will be killed due to exceed memory limit
# $1 - the value of memory.limit_in_bytes
# $2 - the parameters of 'process', such as --shm
# $3 - the -s parameter of 'process', such as 4096
# $4 - use mem+swap limitation
test_proc_kill()
{
	echo $1 > memory.limit_in_bytes
	if [ $4 -eq 1 ]; then
		if [ -e memory.memsw.limit_in_bytes ]; then
			echo $1 > memory.memsw.limit_in_bytes
		else
			tst_resm TCONF "mem+swap is not enabled"
			return
		fi
	fi

	$TEST_PATH/memcg_process $2 -s $3 &
	pid=$!
	sleep 1
	echo $pid > tasks

	kill -s USR1 $pid 2> /dev/null
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
		kill -s INT $pid 2> /dev/null
		result $FAIL "process $pid is not killed"
	fi
}

# Test limit_in_bytes will be aligned to PAGESIZE
# $1 - user input value
# $2 - expected value
# $3 - use mem+swap limitation
test_limit_in_bytes()
{
	echo $1 > memory.limit_in_bytes
	if [ $3 -eq 1 ]; then
		if [ -e memory.memsw.limit_in_bytes ]; then
			echo $1 > memory.memsw.limit_in_bytes
			limit=`cat memory.memsw.limit_in_bytes`
		else
			tst_resm TCONF "mem+swap is not enabled"
			return
		fi
	else
		limit=`cat memory.limit_in_bytes`
	fi

	if [ $limit -eq $2 ]; then
		result $PASS "input=$1, limit_in_bytes=$limit"
	else
		result $FAIL "input=$1, limit_in_bytes=$limit"
	fi
}

# Test memory controller doesn't charge hugepage
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

	kill -s USR1 $! 2> /dev/null
	sleep 1

	check_mem_stat "rss" 0

	echo "TMP_FILE:"
	cat $TMP_FILE

	if [ $4 -eq 0 ]; then
		test -s $TMP_FILE
		if [ $? -eq 0 ]; then
			result $PASS "allocate hugepage failed as expected"
		else
			kill -s USR1 $! 2> /dev/null
			kill -s INT $! 2> /dev/null
			result $FAIL "allocate hugepage shoud fail"
		fi
	else
		test ! -s $TMP_FILE
		if [ $? -eq 0 ]; then
			kill -s USR1 $! 2> /dev/null
			kill -s INT $! 2> /dev/null
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
	kill -s USR1 $! 2> /dev/null
	sleep 1
	check_mem_stat "rss" $PAGESIZE

	cd subgroup
	echo $! > tasks
	check_mem_stat "rss" 0

	# cleanup
	cd ..
	echo $! > tasks
	kill -s INT $! 2> /dev/null
	sleep 1
	rmdir subgroup
}

# Run test cases which test memory.move_charge_at_immigrate
# $1 - the parameters of 'process', such as --shm
# $2 - the -s parameter of 'process', such as 4096
# $3 - some positive value, such as 1
# $4 - the expected size
# $5 - the expected size
test_move_charge()
{
	mkdir subgroup_a

	$TEST_PATH/memcg_process $1 -s $2 &
	sleep 1
	warmup $!
	if [ $? -ne 0 ]; then
		rmdir subgroup_a
		return
	fi

	echo $! > subgroup_a/tasks
	kill -s USR1 $!
	sleep 1

	mkdir subgroup_b
	echo $3 > subgroup_b/memory.move_charge_at_immigrate
	echo $! > subgroup_b/tasks

	cd subgroup_b
	check_mem_stat memory.usage_in_bytes $4
	cd ../subgroup_a
	check_mem_stat memory.usage_in_bytes $5

	cd ..
	echo $! > tasks
	kill -s USR1 $!
	kill -s INT $!
	sleep 1
	rmdir subgroup_a subgroup_b
}

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
	cleanup

	mkdir /dev/memcg 2> /dev/null
	mount -t cgroup -omemory memcg /dev/memcg
}
