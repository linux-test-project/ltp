#! /bin/bash

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
## Author: Li Zefan <lizf@cn.fujitsu.com>                                     ##
##                                                                            ##
################################################################################

cd $LTPROOT/testcases/bin

export TCID="cgroup_regression_test"
export TST_TOTAL=10
export TST_COUNT=1

failed=0

if tst_kvcmp -lt "2.6.29"; then
	tst_brkm TCONF ignored "test must be run with kernel 2.6.29 or newer"
	exit 32
fi

if [ ! -f /proc/cgroups ]; then
	tst_brkm TCONF ignored "Kernel does not support for control groups; skipping testcases";
	exit 32
fi

if [ "x$(id -ru)" != x0 ]; then
	tst_brkm TCONF ignored "Test must be run as root"
	exit 32
fi

dmesg -c > /dev/null
nr_bug=`dmesg | grep -c "kernel BUG"`
nr_null=`dmesg | grep -c "kernel NULL pointer dereference"`
nr_warning=`dmesg | grep -c "^WARNING"`
nr_lockdep=`dmesg | grep -c "possible recursive locking detected"`

# check_kernel_bug - check if some kind of kernel bug happened
check_kernel_bug()
{
	new_bug=`dmesg | grep -c "kernel BUG"`
	new_null=`dmesg | grep -c "kernel NULL pointer dereference"`
	new_warning=`dmesg | grep -c "^WARNING"`
	new_lockdep=`dmesg | grep -c "possible recursive locking detected"`

	# no kernel bug is detected
	if [ $new_bug -eq $nr_bug -a $new_warning -eq $nr_warning -a \
	     $new_null -eq $nr_null -a $new_lockdep -eq $nr_lockdep ]; then
		return 1
	fi

	# some kernel bug is detected
	if [ $new_bug -gt $nr_bug ]; then
		tst_resm TFAIL "kernel BUG was detected!"
	fi
	if [ $new_warning -gt $nr_warning ]; then
		tst_resm TFAIL "kernel WARNING was detected!"
	fi
	if [ $new_null -gt $nr_null ]; then
		tst_resm TFAIL "kernel NULL pointer dereference!"
	fi
	if [ $new_lockdep -gt $nr_lockdep ]; then
		tst_resm TFAIL "kernel lockdep warning was detected!"
	fi

	nr_bug=$new_bug
	nr_null=$new_null
	nr_warning=$new_warning
	nr_lockdep=$new_lockdep

	echo "check_kernel_bug found something!"
	dmesg
	failed=1
	return 0
}

#---------------------------------------------------------------------------
# Bug:    There was a race when keeping forking processes and at the same
#         time cat /cgroup/tasks (should be the very first time to read
#         /cgroup/tasks, otherwise this bug won't be triggered)
# Kernel: 2.6.24, 2.6.25-rcX
# Links:  http://lkml.org/lkml/2007/10/17/224
#         http://lkml.org/lkml/2008/3/5/332
#         http://lkml.org/lkml/2008/4/16/493
# Fix:    commit 0e04388f0189fa1f6812a8e1cb6172136eada87e
#---------------------------------------------------------------------------
test_1()
{
	./fork_processes &
	sleep 1

	mount -t cgroup -o none,name=foo cgroup cgroup/
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount cgroup filesystem"
		failed=1
		/bin/kill -SIGTERM $!
		return
	fi
	cat cgroup/tasks > /dev/null

	check_kernel_bug
	if [ $? -eq 1 ]; then
		tst_resm TPASS "no kernel bug was found"
	fi

	/bin/kill -SIGTERM $!
	wait $!
	umount cgroup/
}

#---------------------------------------------------------------------------
# Bug:    a cgroup's notify_on_release flag did not inherit from its parent.
# Kernel: 2.6.24-rcX
# Links:  http://lkml.org/lkml/2008/2/25/12
# Fix:    commit bc231d2a048010d5e0b49ac7fddbfa822fc41109
#---------------------------------------------------------------------------
test_2()
{
	mount -t cgroup -o none,name=foo cgroup cgroup/
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Failed to mount cgroup filesystem"
		failed=1
		return 1
	fi

	echo 0 > cgroup/notify_on_release
	mkdir cgroup/0
	val1=`cat cgroup/0/notify_on_release`

	echo 1 > cgroup/notify_on_release
	mkdir cgroup/1
	val2=`cat cgroup/1/notify_on_release`

	if [ $val1 -ne 0 -o $val2 -ne 1 ]; then
		tst_resm TFAIL "wrong notify_on_release value"
		failed=1
	else
		tst_resm TPASS "notify_on_release is inherited"
	fi

	rmdir cgroup/0 cgroup/1
	umount cgroup/

	return $failed
}

#---------------------------------------------------------------------------
# Bug:    Accessing NULL cgrp->dentry when reading /proc/sched_debug
# Kernel: 2.6.26-2.6.28
# Links:  http://lkml.org/lkml/2008/10/30/44
#         http://lkml.org/lkml/2008/12/12/107
#         http://lkml.org/lkml/2008/12/16/481
# Fix:    commit a47295e6bc42ad35f9c15ac66f598aa24debd4e2
#---------------------------------------------------------------------------
test_3()
{
	if [ ! -e /proc/sched_debug ]; then
		tst_resm TCONF "CONFIG_SCHED_DEBUG is not enabled"
		return
	fi

	grep -q -w "cpu" /proc/cgroups
	if [ $? -ne 0 ]; then
		tst_resm TCONF "CONFIG_CGROUP_SCHED is not enabled"
		return
	fi

	# Run the test for 30 secs
	mount -t cgroup -o cpu xxx cgroup/
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "Failed to mount cpu subsys"
		failed=1
		return
	fi

	./test_3_1.sh &
	pid1=$!
	./test_3_2.sh &
	pid2=$!

	sleep 30
	/bin/kill -SIGUSR1 $pid1 $pid2
	wait $pid1
	wait $pid2

	check_kernel_bug
	if [ $? -eq 1 ]; then
		tst_resm TPASS "no kernel bug was found"
	fi

	rmdir cgroup/* 2> /dev/null
	umount cgroup/
}

#---------------------------------------------------------------------------
# Bug:    cgroup hierarchy lock's lockdep subclass may overflow
# Kernel: 2.6.29-rcX
# Link:   http://lkml.org/lkml/2009/2/4/67
# Fix:
#---------------------------------------------------------------------------
test_4()
{
	if [ ! -e /proc/lockdep ]; then
		tst_resm TCONF "CONFIG_LOCKDEP is not enabled"
		return
	fi

	# MAX_LOCKDEP_SUBCLASSES is 8, so number of subsys should be > 8
	lines=`cat /proc/cgroups | wc -l`
	if [ $lines -le 9 ]; then
		tst_resm TCONF "require more than 8 cgroup subsystems"
		return
	fi

	mount -t cgroup -o none,name=foo cgroup cgroup/
	mkdir cgroup/0
	rmdir cgroup/0
	umount cgroup/

	dmesg | grep -q "MAX_LOCKDEP_SUBCLASSES too low"
	if [ $? -eq 0 ]; then
		tst_resm TFAIL "lockdep BUG was found"
		failed=1
		return
	else
		tst_resm TPASS "no lockdep BUG was found"
	fi
}

#---------------------------------------------------------------------------
# Bug:    When mount cgroup fs and the fs was busy, root_count should not be
#         decremented in cgroup_kill_sb()
# Kernel: 2.6.29-rcX
# Links:  https://openvz.org/pipermail/devel/2009-January/016345.html
#         http://lkml.org/lkml/2009/1/28/190
# Fix:    commit 839ec5452ebfd5905b9c69b20ceb640903a8ea1a
#---------------------------------------------------------------------------
test_5()
{
	lines=`cat /proc/cgroups | wc -l`
	if [ $lines -le 2 ]; then
		tst_resm TCONF "require at least 2 cgroup subsystems"
		return
	fi

	subsys1=`tail -n 1 /proc/cgroups | awk '{ print $1 }'`
	subsys2=`tail -n 2 /proc/cgroups | head -1 | awk '{ print $1 }'`

	mount -t cgroup -o $subsys1,$subsys2 xxx cgroup/
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "mount $subsys1 and $subsys2 failed"
		failed=1
		return
	fi

	# This 2nd mount should fail
	mount -t cgroup -o $subsys1 xxx cgroup/ 2> /dev/null
	if [ $? -eq 0 ]; then
		tst_resm TFAIL "mount $subsys1 should fail"
		umount cgroup/
		failed=1
		return
	fi

	mkdir cgroup/0
	# Otherwise we can't attach task
	if [ "$subsys1" = cpuset -o "$subsys2" = cpuset ]; then
		echo 0 > cgroup/0/cpuset.cpus 2> /dev/null
		echo 0 > cgroup/0/cpuset.mems 2> /dev/null
	fi

	sleep 100 &
	echo $! > cgroup/0/tasks

	check_kernel_bug
	if [ $? -eq 1 ]; then
		tst_resm TPASS "no kernel bug was found"
	fi

	# clean up
	/bin/kill -SIGTERM $! > /dev/null
	wait $!
	rmdir cgroup/0
	umount cgroup/
}

#---------------------------------------------------------------------------
# Bug:    There was a race between cgroup_clone and umount
# Kernel: 2.6.24 - 2.6.28, 2.6.29-rcX
# Links:  http://lkml.org/lkml/2008/12/24/124
# Fix:    commit 7b574b7b0124ed344911f5d581e9bc2d83bbeb19
#---------------------------------------------------------------------------
test_6()
{
	grep -q -w "ns" /proc/cgroups
	if [ $? -ne 0 ]; then
		tst_resm TCONF "CONFIG_CGROUP_NS"
		return
	fi

	# run the test for 30 secs
	./test_6_1.sh &
	pid1=$!
	./test_6_2 &
	pid2=$!

	sleep 30
	/bin/kill -SIGUSR1 $pid1
	/bin/kill -SIGTERM $pid2
	wait $pid1
	wait $pid2

	check_kernel_bug
	if [ $? -eq 1 ]; then
		tst_resm TPASS "no kernel bug was found"
	fi

	# clean up
	mount -t cgroup -o ns xxx cgroup/ > /dev/null 2>&1
	rmdir cgroup/[1-9]* > /dev/null 2>&1
	umount cgroup/
}

#---------------------------------------------------------------------------
# Bug:    There was a bug when remount cgroup fs with some dead subdirs in
#         it (rmdir()ed but still has some refcnts on it). It caused memory
#         leak, and may cause oops when cat /proc/sched_debug.
# Kernel: 2.6.24 - 2.6.27, 2.6.28-rcX
# Links:  http://lkml.org/lkml/2008/12/10/369
# Fix:    commit 307257cf475aac25db30b669987f13d90c934e3a
#---------------------------------------------------------------------------
test_7_1()
{
	mount -t cgroup -o $subsys xxx cgroup/
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount $subsys"
		failed=1
		return
	fi

	mkdir cgroup/0
	sleep 100 < cgroup/0 &	# add refcnt to this dir
	rmdir cgroup/0

	# remount with new subsystems added
	# since 2.6.28, this remount will fail
	mount -t cgroup -o remount xxx cgroup/ 2> /dev/null
	/bin/kill -SIGTERM $!
	wait $!
	umount cgroup/
}

test_7_2()
{
	mount -t cgroup -o none,name=foo cgroup cgroup/
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount cgroup"
		failed=1
		return
	fi

	mkdir cgroup/0
	sleep 100 < cgroup/0 &	# add refcnt to this dir
	rmdir cgroup/0

	# remount with some subsystems removed
	# since 2.6.28, this remount will fail
	mount -t cgroup -o remount,$subsys xxx cgroup/ 2> /dev/null
	/bin/kill -SIGTERM $!
	wait $!
	umount cgroup/

	# due to the bug, reading /proc/sched_debug may lead to oops
	grep -q -w "cpu" /proc/cgroups
	if [ $? -ne 0 -o ! -e /proc/sched_debug ]; then
		return
	fi

	tmp=0
	while [ $tmp -lt 50 ] ; do
		echo 3 > /proc/sys/vm/drop_caches
		cat /proc/sched_debug > /dev/null
		: $(( tmp += 1 ))
	done
}

test_7()
{
	lines=`cat /proc/cgroups | wc -l`
	if [ $lines -le 2 ]; then
		tst_resm TCONF "require at least 2 cgroup subsystems"
		slt_result $SLT_Untested
		return
	fi

	subsys=`tail -n 1 /proc/cgroups | awk '{ print $1 }'`

	# remount to add new subsystems to the hierarchy
	i=1
	while [ $i -le 2 ] ; do
		test_7_$i
		if [ $? -ne 0 ]; then
			return
		fi

		check_kernel_bug
		if [ $? -eq 0 ]; then
			return
		fi
		: $(( i += 1 ))
	done

	tst_resm TPASS "no kernel bug was found"
}

#---------------------------------------------------------------------------
# Bug:    oops when get cgroupstat of a cgroup control file
# Kernel: 2.6.24 - 2.6.27, 2.6.28-rcX
# Links:  http://lkml.org/lkml/2008/11/19/53
# Fix:    commit 33d283bef23132c48195eafc21449f8ba88fce6b
#---------------------------------------------------------------------------
test_8()
{
	mount -t cgroup -o none,name=foo cgroup cgroup/
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount cgroup filesystem"
		failed=1
		return
	fi

	./getdelays -C cgroup/tasks > /dev/null 2>&1
	if [ $? -eq 0 ]; then
		tst_resm TFAIL "should have failed to get cgroupstat of tasks file"
		umount cgroup/
		failed=1
		return
	fi

	check_kernel_bug
	if [ $? -eq 1 ]; then
		tst_resm TPASS "no kernel bug was found"
	fi

	umount cgroup/
}

#---------------------------------------------------------------------------
# Bug:    When running 2 concurrent mount/umount threads, lockdep warning
#         may be triggered, it's a false positive, and it's VFS' issue but
#         not cgroup.
# Kernel: 2.6.24 - 2.6.29-rcX
# Links:  http://lkml.org/lkml/2009/1/4/352
# Fix:    commit ada723dcd681e2dffd7d73345cc8fda0eb0df9bd
#---------------------------------------------------------------------------
test_9()
{
	./test_9_1.sh &
	pid1=$!
	./test_9_2.sh &
	pid2=$!

	sleep 30
	/bin/kill -SIGUSR1 $pid1 $pid2
	wait $pid1
	wait $pid2

	umount cgroup/ 2> /dev/null

	check_kernel_bug
	if [ $? -eq 1 ]; then
		tst_resm TPASS "no kernel warning was found"
	fi
}

#---------------------------------------------------------------------------
# Bug:    When running 2 concurrent mount/umount threads, kernel WARNING
#         may be triggered, but it's VFS' issue but not cgroup.
# Kernel: 2.6.24 - 2.6.29-rcX
# Links:  http://lkml.org/lkml/2009/1/4/354
# Fix:    commit 1a88b5364b535edaa321d70a566e358390ff0872
#---------------------------------------------------------------------------
test_10()
{
	./test_10_1.sh &
	pid1=$!
	./test_10_2.sh &
	pid2=$!

	sleep 30
	/bin/kill -SIGUSR1 $pid1 $pid2
	wait $pid1
	wait $pid2

	mount -t cgroup none cgroup 2> /dev/null
	rmdir cgroup/0
	umount cgroup/

	check_kernel_bug
	if [ $? -eq 1 ]; then
		tst_resm TPASS "no kernel warning was found"
	fi
}

# main

mkdir cgroup/

for ((cur = 1; cur <= $TST_TOTAL; cur++))
{
	export TST_COUNT=$cur

	test_$cur
}

find cgroup/ -maxdepth 1 -depth -exec rmdir {} +

exit $failed
