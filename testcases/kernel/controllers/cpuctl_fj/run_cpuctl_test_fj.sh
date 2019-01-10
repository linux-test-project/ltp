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
## Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    ##
##                                                                            ##
## Author: Miao Xie <miaox@cn.fujitsu.com>                                    ##
## Restructure for LTP: Shi Weihua <shiwh@cn.fujitsu.com>                     ##
##                                                                            ##
################################################################################

cd $LTPROOT/testcases/bin

export TCID="cpuctl_test_fj"
export TST_TOTAL=22
export TST_COUNT=1

CPUCTL="/dev/cpuctl"
CPUCTL_TMP="/tmp/cpuctl_tmp"
SLEEP_SEC=5

# Create $CPUCTL &  mount the cgroup file system with cpu controller
# clean any group created earlier (if any)
setup()
{
	if [ -e $CPUCTL ]
	then
		tst_resm TWARN "WARN:$CPUCTL already exist..overwriting"
		cleanup || {
			tst_resm TFAIL "Err: Can't cleanup... Exiting"
			exit -1;
		}
	fi

	mkdir -p "$CPUCTL" || return 1

	mount -t cgroup -o cpu cpuctl "$CPUCTL" || {
		tst_resm TFAIL "failed to mount cpu subsystem... Exiting"
		cleanup;
		return 1;
	}
}

# Write the cleanup function
cleanup()
{
	mount | grep "$CPUCTL" > /dev/null 2>&1 || {
		rm -rf "$CPUCTL" > /dev/null 2>&1
		return 0
	}

	find $CPUCTL -type d | sort | sed -n '2,$p' | tac | while read tmpdir
	do
		while read tmppid
		do
			echo $tmppid > $CPUCTL/tasks
		done  < $tmpdir/tasks
		rmdir $tmpdir || return 1
	done

	umount $CPUCTL || return 1
	rmdir $CPUCTL > /dev/null 2>&1
}

creat_process()
{
	cat /dev/zero > /dev/null 2>/dev/null &
	taskset -p 1 $! >/dev/null 2>&1
	echo $!
}

get_cpu_usage()
{
	# XXX (garrcoop): expr(1) can't do float point comparisons
	# apparently...
	#
	# gcooper@optimus ~ $ expr 0 \< 42 \& 42 \< 100
	# 1
	# gcooper@optimus ~ $ expr 0 \< 42.0 \& 42.0 \< 100
	# 0
	# gcooper@optimus ~ $ expr 0.0 \< 42.0 \& 42.0 \< 100.0
	# 0
	# ... so we have to lop off the fractional piece.
	# ps -p $1 pcpu | awk -F. '{ print $1 }'
	top -bn1 -p $1 | sed -n "8p" | awk '{ print $9 }' | awk -F. '{ print $1 }'
}

kill_all_pid()
{
	while read pid_to_be_killed
	do
		kill -9 $pid_to_be_killed
	done
}

TESTUSER="`whoami`"
if [ "$TESTUSER" != "root" ]
then
	tst_brkm TBROK ignored "Test must be run as root"
	exit 0
fi

# only root directory
case1 ()
{
	[ -f "$CPUCTL/cpu.shares" ] || {
		tst_resm TFAIL "Err: No cpu.shares."
		return 1
	}

	shares=`cat $CPUCTL/cpu.shares`
	if [ $shares -ne 1024 ]
	then
		tst_resm TFAIL "Err: Init value is not 1024"
		return 1;
	fi

	ps -eo pid,rtprio > /tmp/pids_file1 &
	pspid=$!
	wait $pspid
	cat /tmp/pids_file1 | grep '-' | tr -d '-' | tr -d ' ' | \
	grep -v "$pspid" > /tmp/pids_file2

	while read pid
	do
		task=`cat $CPUCTL/tasks | grep "\b$pid\b"`
		if [ -z $task ]
		then
			tst_resm TFAIL  "Err: Some normal tasks aren't in the root group"
			return 1
		fi
	done < /tmp/pids_file2
}

# create a child directory
case2 ()
{
	mkdir $CPUCTL/tmp
	if [ $? -ne 0 ]
	then
		return 1;
	fi

	[ -d "$CPUCTL/tmp" ] || return 1

	[ -f "$CPUCTL/tmp/cpu.shares" ] || return 1

	shares=`cat $CPUCTL/tmp/cpu.shares`
	if [ $shares -ne 1024 ]
	then
		return 1;
	fi

	task=`cat $CPUCTL/tmp/tasks`
	if [ "$task" != "" ]
	then
		return 1
	fi
}

# create a grand-directory
case3 ()
{
	mkdir $CPUCTL/tmp
	if [ $? -ne 0 ]
	then
		return 1;
	fi

	mkdir $CPUCTL/tmp/tmp1
	if [ $? -ne 0 ]
	then
		return 1;
	fi

	[ -d "$CPUCTL/tmp/tmp1" ] || return 1

	[ -f "$CPUCTL/tmp/tmp1/cpu.shares" ] || return 1

	shares=`cat $CPUCTL/tmp/tmp1/cpu.shares`
	if [ $shares -ne 1024 ]
	then
		return 1;
	fi

	task=`cat $CPUCTL/tmp/tmp1/tasks`
	if [ "$task" != "" ]
	then
		return 1
	fi
}

# attach general process
case4 ()
{
	mkdir $CPUCTL/tmp
	if [ $? -ne 0 ]
	then
		return 1;
	fi

	echo 1 > $CPUCTL/tmp/tasks
	if [ $? -ne 0 ]
	then
		return 1;
	fi

	tasks=`cat $CPUCTL/tmp/tasks`
	if [ $tasks -ne 1 ]
	then
		return 1;
	fi
}

# attach realtime process
case5 ()
{
	mkdir $CPUCTL/tmp
	if [ $? -ne 0 ]
	then
		return 1;
	fi

	./cpuctl_fj_simple_echo 3 $CPUCTL/tmp/tasks
	if [ $? -ne 22 ]	# define EINVAL 22  /* Invalid argument */
	then
		return 1;
	fi

	tasks=`cat $CPUCTL/tmp/tasks`
	if [ "$tasks" != "" ]
	then
		return 1;
	fi
}

# modify the shares of the root group
case6 ()
{
	./cpuctl_fj_simple_echo 2048 $CPUCTL/cpu.shares
	if [ $? -ne 22 ]	# define EINVAL 22  /* Invalid argument */
	then
		return 1;
	fi

	shares=`cat $CPUCTL/cpu.shares`
	if [ $shares -ne 1024 ]
	then
		return 1;
	fi
}

# echo negative into shares
case7 ()
{
	mkdir $CPUCTL/tmp

	./cpuctl_fj_simple_echo -1 $CPUCTL/tmp/cpu.shares
	if [ $? -ne 22 ]	# define EINVAL 22  /* Invalid argument */
	then
		return 1;
	fi

	shares=`cat $CPUCTL/tmp/cpu.shares`
	if [ $shares -ne 1024 ]
	then
		return 1;
	fi

	./cpuctl_fj_cpu-hog &
	pid=$!

	echo $pid > $CPUCTL/tmp/tasks

	/bin/kill -s SIGUSR1 $pid
	sleep $SLEEP_SEC
	/bin/kill -s SIGUSR1 $pid
	wait $pid
}

# echo 0 into shares
case8 ()
{
	mkdir $CPUCTL/tmp

	echo 0 > $CPUCTL/tmp/cpu.shares
	if [ $? -ne 0 ]
	then
		return 1;
	fi

	shares=`cat $CPUCTL/tmp/cpu.shares`
	if [ $shares -ne 2 ]
	then
		return 1;
	fi

	./cpuctl_fj_cpu-hog &
	pid=$!

	echo $pid > $CPUCTL/tmp/tasks

	/bin/kill -s SIGUSR1 $pid
	sleep $SLEEP_SEC
	/bin/kill -s SIGUSR1 $pid
	wait $pid
}

# echo 1 into shares
case9 ()
{
	mkdir $CPUCTL/tmp

	echo 1 > $CPUCTL/tmp/cpu.shares
	if [ $? -ne 0 ]
	then
		return 1;
	fi

	shares=`cat $CPUCTL/tmp/cpu.shares`
	if [ $shares -ne 2 ]
	then
		return 1;
	fi

	./cpuctl_fj_cpu-hog &
	pid=$!

	echo $pid > $CPUCTL/tmp/tasks

	/bin/kill -s SIGUSR1 $pid
	sleep $SLEEP_SEC
	/bin/kill -s SIGUSR1 $pid
	wait $pid
}

# echo 2 into shares
case10 ()
{
	mkdir $CPUCTL/tmp

	echo 2 > $CPUCTL/tmp/cpu.shares
	if [ $? -ne 0 ]
	then
		return 1;
	fi

	shares=`cat $CPUCTL/tmp/cpu.shares`
	if [ $shares -ne 2 ]
	then
		return 1;
	fi

	./cpuctl_fj_cpu-hog &
	pid=$!

	echo $pid > $CPUCTL/tmp/tasks

	/bin/kill -s SIGUSR1 $pid
	sleep $SLEEP_SEC
	/bin/kill -s SIGUSR1 $pid
	wait $pid
}

# echo 3 into shares
case11 ()
{
	mkdir $CPUCTL/tmp

	echo 3 > $CPUCTL/tmp/cpu.shares
	if [ $? -ne 0 ]
	then
		return 1;
	fi

	shares=`cat $CPUCTL/tmp/cpu.shares`
	if [ $shares -ne 3 ]
	then
		return 1;
	fi

	./cpuctl_fj_cpu-hog &
	pid=$!

	echo $pid > $CPUCTL/tmp/tasks

	/bin/kill -s SIGUSR1 $pid
	sleep $SLEEP_SEC
	/bin/kill -s SIGUSR1 $pid
	wait $pid
}

# echo 2048 into shares
case12 ()
{
	mkdir $CPUCTL/tmp

	echo 2048 > $CPUCTL/tmp/cpu.shares
	if [ $? -ne 0 ]
	then
		return 1;
	fi

	shares=`cat $CPUCTL/tmp/cpu.shares`
	if [ $shares -ne 2048 ]
	then
		return 1;
	fi

	./cpuctl_fj_cpu-hog &
	pid=$!

	echo $pid > $CPUCTL/tmp/tasks

	/bin/kill -s SIGUSR1 $pid
	sleep $SLEEP_SEC
	/bin/kill -s SIGUSR1 $pid
	wait $pid
}

max_shares=$((1 << 18))

# echo MAX_SHARES into shares
case13 ()
{
	mkdir $CPUCTL/tmp

	echo $max_shares > $CPUCTL/tmp/cpu.shares
	if [ $? -ne 0 ]
	then
		return 1;
	fi

	shares=`cat $CPUCTL/tmp/cpu.shares`
	if [ "$shares" != "$max_shares" ]
	then
		return 1;
	fi

	./cpuctl_fj_cpu-hog &
	pid=$!

	echo $pid > $CPUCTL/tmp/tasks

	/bin/kill -s SIGUSR1 $pid
	sleep $SLEEP_SEC
	/bin/kill -s SIGUSR1 $pid
	wait $pid
}

# echo MAX_SHARES+1 into shares
case14 ()
{
	mkdir $CPUCTL/tmp

	echo $(($max_shares+1)) > $CPUCTL/tmp/cpu.shares
	if [ $? -ne 0 ]
	then
		return 1;
	fi

	shares=`cat $CPUCTL/tmp/cpu.shares`
	if [ "$shares" != "$max_shares" ]
	then
		return 1;
	fi

	./cpuctl_fj_cpu-hog &
	pid=$!

	echo $pid > $CPUCTL/tmp/tasks

	/bin/kill -s SIGUSR1 $pid
	sleep $SLEEP_SEC
	/bin/kill -s SIGUSR1 $pid
	wait $pid
}

# echo float number into shares
case15 ()
{
	mkdir $CPUCTL/tmp

	./cpuctl_fj_simple_echo 2048.23 $CPUCTL/tmp/cpu.shares
	if [ $? -ne 22 ]	# define EINVAL 22  /* Invalid argument */
	then
		return 1;
	fi

	shares=`cat $CPUCTL/tmp/cpu.shares`
	if [ $shares -ne 1024 ]
	then
		return 1;
	fi

	./cpuctl_fj_cpu-hog &
	pid=$!

	echo $pid > $CPUCTL/tmp/tasks

	/bin/kill -s SIGUSR1 $pid
	sleep $SLEEP_SEC
	/bin/kill -s SIGUSR1 $pid
	wait $pid
}

# echo a string into shares. This string begins with some number, and ends with
# charactor.
case16 ()
{
	mkdir $CPUCTL/tmp

	./cpuctl_fj_simple_echo 2048ABC $CPUCTL/tmp/cpu.shares
	if [ $? -ne 22 ]	# define EINVAL 22  /* Invalid argument */
	then
		return 1;
	fi

	shares=`cat $CPUCTL/tmp/cpu.shares`
	if [ $shares -ne 1024 ]
	then
		return 1;
	fi

	./cpuctl_fj_cpu-hog &
	pid=$!

	echo $pid > $CPUCTL/tmp/tasks

	/bin/kill -s SIGUSR1 $pid
	sleep $SLEEP_SEC
	/bin/kill -s SIGUSR1 $pid
	wait $pid
}

# echo a string into shares. This string begins with charactors.
case17 ()
{
	mkdir $CPUCTL/tmp

	./cpuctl_fj_simple_echo ABC $CPUCTL/tmp/cpu.shares
	if [ $? -ne 22 ]	 # define EINVAL 22  /* Invalid argument */
	then
		return 1;
	fi

	shares=`cat $CPUCTL/tmp/cpu.shares`
	if [ $shares -ne 1024 ]
	then
		return 1;
	fi

	./cpuctl_fj_cpu-hog &
	pid=$!

	echo $pid > $CPUCTL/tmp/tasks

	/bin/kill -s SIGUSR1 $pid
	sleep $SLEEP_SEC
	/bin/kill -s SIGUSR1 $pid
	wait $pid
}

case18()
{
	mkdir "$CPUCTL/1"
	echo 0x8000000000000000 > "$CPUCTL/1/cpu.shares"
	pid=$(creat_process)
	echo $pid > "$CPUCTL/1/tasks"
	kill -9 $pid
	return 0
}

case19()
{
	mkdir "$CPUCTL/1"
	pid=$(creat_process)
	pid_other=$(creat_process)

	echo 2000000000 > "$CPUCTL/1/cpu.shares"
	echo $pid > "$CPUCTL/1/tasks"
	cpu_usage=$(get_cpu_usage $pid)
	echo "pid $pid cpu_usage $cpu_usage"

	kill -9 $pid $pid_other
	expr 96 \< "$cpu_usage" \& "$cpu_usage" \< 103 > /dev/null 2>&1 || return 1
	return 0
}

case20()
{
	mkdir "$CPUCTL/1"
	pid=$(creat_process)
	pid_other=$(creat_process)

	echo 20000000000 > "$CPUCTL/1/cpu.shares"
	echo $pid > "$CPUCTL/1/tasks"
	cpu_usage=$(get_cpu_usage $pid)
	echo "pid $pid cpu_usage $cpu_usage"

	kill -9 $pid $pid_other
	expr 96 \< "$cpu_usage" \& "$cpu_usage" \< 103 > /dev/null 2>&1 || return 1
	return 0
}

case21()
{
	mkdir "$CPUCTL/1" "$CPUCTL/1/2"
	pid=$(creat_process)
	echo $pid > "$CPUCTL/1/tasks"

	while true
	do
		creat_process > "$CPUCTL/1/2/tasks"
		sleep 1
	done &
	loop_pid=$!

	sleep 10
	ret=0

	top_times=0
	while [ "$top_times" -lt 10 -a "$ret" = 0 ]
	do
		cpu_usage=$(get_cpu_usage $pid)
		echo "pid $pid cpu_usage $cpu_usage"
		expr 44 \< "$cpu_usage" \& "$cpu_usage" \< 56 > /dev/null 2>&1
		ret=$?
		: $(( top_times+=1 ))
	done

	kill -9 $pid $loop_pid > /dev/null 2>&1
	wait $pid $loop_pid >/dev/null 2>&1
	sleep 2
	kill_all_pid < "$CPUCTL/1/2/tasks"  >/dev/null 2>&1
	sleep 2
	return $ret
}

case22()
{
	mkdir "$CPUCTL/1" "$CPUCTL/1/2"
	pid=$(creat_process)
	while true
	do
		echo $pid > "$CPUCTL/1/tasks"
		echo $pid > "$CPUCTL/1/2/tasks"
		sleep 1
	done &
	loop_pid=$!

	sleep 10
	ret=0

	top_times=0
	while [ "$top_times" -lt 10 -a "$ret" = 0 ]
	do
		cpu_usage=$(get_cpu_usage $pid)
		echo "pid $pid cpu_usage $cpu_usage"
		expr 94 \< "$cpu_usage" \& "$cpu_usage" \< 106 > /dev/null 2>&1
		ret=$?
		: $(( top_times+=1 ))
	done

	kill -s KILL $pid $loop_pid > /dev/null 2>&1
	wait $pid $loop_pid >/dev/null 2>&1
	return $ret
}

rm -rf "$CPUCTL_TMP" >/dev/null 2>&1 || exit 1
mkdir -p "$CPUCTL_TMP" || exit 1

# test
do_test ()
{
	for i in $(seq 1 $TST_TOTAL)
	do
		setup || {
			tst_resm TFAIL "case$i    FAIL"
			continue
		}

		case$i || {
			tst_resm TFAIL "case$i    FAIL"
			cleanup
			continue
		}

		cleanup || {
			tst_resm TFAIL "case$i    FAIL"
		}

		tst_resm TPASS "case$i    PASS"
	done
}

do_test

rm -rf "$CPUCTL_TMP" >/dev/null 2>&1

