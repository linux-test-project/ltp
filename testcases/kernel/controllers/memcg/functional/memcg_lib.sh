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
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
## Author: Peng Haitao <penght@cn.fujitsu.com>                                ##
##                                                                            ##
################################################################################

TST_NEEDS_CHECKPOINTS=1
. test.sh

if [ "x$(grep -w memory /proc/cgroups | cut -f4)" != "x1" ]; then
	tst_brkm TCONF "Kernel does not support the memory resource controller"
fi

PAGESIZE=$(getconf PAGESIZE)
if [ $? -ne 0 ]; then
	tst_brkm TBROK "getconf PAGESIZE failed"
fi

HUGEPAGESIZE=$(awk '/Hugepagesize/ {print $2}' /proc/meminfo)
[ -z $HUGEPAGESIZE ] && HUGEPAGESIZE=0
HUGEPAGESIZE=$(( $HUGEPAGESIZE * 1024 ))
orig_memory_use_hierarchy=""

MEMSW_USAGE_FLAG=0
MEMSW_LIMIT_FLAG=0

tst_tmpdir
TMP_DIR="$PWD"

cleanup()
{
	if [ -n "$LOCAL_CLEANUP" ]; then
		$LOCAL_CLEANUP
	fi

	killall -9 memcg_process 2> /dev/null
	wait

	cd "$TMP_DIR"

	if [ -n "$TEST_ID" -a -d "/dev/memcg/$TEST_ID" ]; then
		rmdir "/dev/memcg/$TEST_ID"
	fi

	if [ -d "/dev/memcg" ]; then
		umount /dev/memcg
		rmdir /dev/memcg
	fi

	tst_rmdir
}
TST_CLEANUP=cleanup

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
		tst_resm TPASS "$1 is $2 as expected"
	else
		tst_resm TFAIL "$1 is $item_size, $2 expected"
	fi
}

warmup()
{
	pid=$1

	tst_resm TINFO "Warming up pid: $pid"
	kill -s USR1 $pid 2> /dev/null
	sleep 1
	kill -s USR1 $pid 2> /dev/null
	sleep 1

	kill -0 $pid
	if [ $? -ne 0 ]; then
		wait $pid
		tst_resm TFAIL "Process $pid exited with $? after warm up"
		return 1
	else
		tst_resm TINFO "Process is still here after warm up: $pid"
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
	tst_resm TINFO "Running memcg_process $1 -s $2"
	memcg_process $1 -s $2 &
	TST_CHECKPOINT_WAIT 0

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
	tst_resm TINFO "Running memcg_process $1 -s $2"
	memcg_process $1 -s $2 &
	TST_CHECKPOINT_WAIT 0

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
	tst_resm TINFO "Running memcg_process $1 -s $2"
	memcg_process $1 -s $2 &
	TST_CHECKPOINT_WAIT 0

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
		tst_resm TPASS "$1 is $failcnt, > 0 as expected"
	else
		tst_resm TFAIL "$1 is $failcnt, <= 0 expected"
	fi
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

	memcg_process $2 -s $3 &
	pid=$!
	TST_CHECKPOINT_WAIT 0
	echo $pid > tasks

	kill -s USR1 $pid 2> /dev/null

	tpk_pid_exists=1
	for tpk_iter in $(seq 20); do
		if [ ! -d "/proc/$pid" ] ||
			grep -q 'Z (zombie)' "/proc/$pid/status"; then
			tpk_pid_exists=0
			break
		fi

		tst_sleep 250ms
	done

	if [ $tpk_pid_exists -eq 0 ]; then
		wait $pid
		ret=$?
		if [ $ret -eq 1 ]; then
			tst_resm TFAIL "process $pid is killed by error"
		elif [ $ret -eq 2 ]; then
			tst_resm TFAIL "Failed to lock memory"
		else
			tst_resm TPASS "process $pid is killed"
		fi
	else
		kill -s INT $pid 2> /dev/null
		tst_resm TFAIL "process $pid is not killed"
	fi
}

# Test limit_in_bytes will be aligned to PAGESIZE
# $1 - user input value
# $2 - use mem+swap limitation
test_limit_in_bytes()
{
	echo $1 > memory.limit_in_bytes
	if [ $2 -eq 1 ]; then
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

	# Kernels prior to 3.19 were rounding up but newer kernels
	# are rounding down
	if [ \( $(($PAGESIZE*($1/$PAGESIZE))) -eq $limit \) \
	    -o \( $(($PAGESIZE*(($1+$PAGESIZE-1)/$PAGESIZE))) -eq $limit \) ]; then
		tst_resm TPASS "input=$1, limit_in_bytes=$limit"
	else
		tst_resm TFAIL "input=$1, limit_in_bytes=$limit"
	fi
}

# Never used, so untested
#
# Test memory controller doesn't charge hugepage
# $1 - the value of /proc/sys/vm/nr_hugepages
# $2 - the parameters of 'process', --mmap-file or --shm
# $3 - the -s parameter of 'process', such as $HUGEPAGESIZE
# $4 - 0: expected failure, 1: expected success
test_hugepage()
{
	TMP_FILE="$TMP_DIR/tmp"
	nr_hugepages=`cat /proc/sys/vm/nr_hugepages`

	mkdir /hugetlb
	mount -t hugetlbfs none /hugetlb

	echo $1 > /proc/sys/vm/nr_hugepages

	memcg_process $2 --hugepage -s $3 > $TMP_FILE 2>&1 &
	TST_CHECKPOINT_WAIT 0

	kill -s USR1 $! 2> /dev/null
	sleep 1

	check_mem_stat "rss" 0

	echo "TMP_FILE:"
	cat $TMP_FILE

	if [ $4 -eq 0 ]; then
		test -s $TMP_FILE
		if [ $? -eq 0 ]; then
			tst_resm TPASS "allocate hugepage failed as expected"
		else
			kill -s USR1 $! 2> /dev/null
			kill -s INT $! 2> /dev/null
			tst_resm TFAIL "allocate hugepage should fail"
		fi
	else
		test ! -s $TMP_FILE
		if [ $? -eq 0 ]; then
			kill -s USR1 $! 2> /dev/null
			kill -s INT $! 2> /dev/null
			tst_resm TPASS "allocate hugepage succeeded"
		else
			tst_resm TFAIL "allocate hugepage failed"
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

	tst_resm TINFO "Running memcg_process --mmap-anon -s $PAGESIZE"
	memcg_process --mmap-anon -s $PAGESIZE &
	TST_CHECKPOINT_WAIT 0

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

	tst_resm TINFO "Running memcg_process $1 -s $2"
	memcg_process $1 -s $2 &
	TST_CHECKPOINT_WAIT 0
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
	check_mem_stat "rss" $4
	check_mem_stat "cache" $5
	cd ../subgroup_a
	check_mem_stat "rss" $6
	check_mem_stat "cache" $7

	cd ..
	echo $! > tasks
	kill -s USR1 $!
	kill -s INT $!
	sleep 1
	rmdir subgroup_a subgroup_b
}

cleanup_test()
{
	TEST_ID="$1"

	if [ -n "$orig_memory_use_hierarchy" ];then
		echo $orig_memory_use_hierarchy > \
		     /dev/memcg/memory.use_hierarchy
		if [ $? -ne 0 ];then
			tst_resm TINFO "restore "\
				 "/dev/memcg/memory.use_hierarchy failed"
		fi
		orig_memory_use_hierarchy=""
	fi

	killall -9 memcg_process 2>/dev/null
	wait

	ROD cd "$TMP_DIR"

	ROD rmdir "/dev/memcg/$TEST_ID"
	TEST_ID=""
	ROD umount /dev/memcg
	ROD rmdir /dev/memcg
}

setup_test()
{
	TEST_ID="$1"

	ROD mkdir /dev/memcg
	ROD mount -t cgroup -omemory memcg /dev/memcg

	# The default value for memory.use_hierarchy is 0 and some of tests
	# (memcg_stat_test.sh and memcg_use_hierarchy_test.sh) expect it so
	# while there are distributions (RHEL7U0Beta for example) that sets
	# it to 1.
	orig_memory_use_hierarchy=$(cat /dev/memcg/memory.use_hierarchy)
	if [ -z "orig_memory_use_hierarchy" ];then
		tst_resm TINFO "cat /dev/memcg/memory.use_hierarchy failed"
	elif [ "$orig_memory_use_hierarchy" = "0" ];then
		orig_memory_use_hierarchy=""
	else
		echo 0 > /dev/memcg/memory.use_hierarchy
		if [ $? -ne 0 ];then
			tst_resm TINFO "set /dev/memcg/memory.use_hierarchy" \
				"to 0 failed"
		fi
	fi

	ROD mkdir "/dev/memcg/$TEST_ID"
	ROD cd "/dev/memcg/$TEST_ID"
}

# Run all the test cases
run_tests()
{
	for i in $(seq 1 $TST_TOTAL); do
		setup_test $i

		if [ -e memory.memsw.limit_in_bytes ]; then
			MEMSW_LIMIT_FLAG=1
		fi

		if [ -e memory.memsw.max_usage_in_bytes ]; then
			MEMSW_USAGE_FLAG=1
		fi

		testcase_$i

		cleanup_test $i
	done
}
