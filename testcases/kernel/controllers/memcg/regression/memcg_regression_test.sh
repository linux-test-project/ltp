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
## Author: Li Zefan <lizf@cn.fujitsu.com>                                     ##
## Added memcg enable/disable functinality: Rishikesh K Rajak                 ##
##                                              <risrajak@linux.vnet.ibm.com  ##
##                                                                            ##
################################################################################

cd $LTPROOT/testcases/bin

export TCID="memcg_regression_test"
export TST_TOTAL=4
export TST_COUNT=1

if [ "$(id -ru)" != 0 ]; then
	tst_brkm TBROK ignored "Test must be run as root"
	exit 0
fi

if [ "x$(grep -w memory /proc/cgroups | cut -f4)" != "x1" ]; then
	tst_resm TCONF "Either memory resource controller kernel support absent"
	tst_resm TCONF "or feature is not enabled; skipping all memcgroup testcases."
        exit 0
fi

if tst_kvcmp -lt "2.6.30"; then
	tst_brkm TBROK ignored "Test should be run with kernel 2.6.30 or newer"
	exit 0
fi

#buffer can rotate and number of found bugs can actually go down
#so clear the buffer to avoid this
dmesg -c > /dev/null
nr_bug=0
nr_null=0
nr_warning=0
nr_lockdep=0

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
		tst_resm "kernel NULL pointer dereference!"
	fi
	if [ $new_lockdep -gt $nr_lockdep ]; then
		tst_resm "kernel lockdep warning was detected!"
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
# Bug:    The bug was, while forking mass processes, trigger memcgroup OOM,
#         then NULL pointer dereference may be hit.
# Kernel: 2.6.25-rcX
# Links:  http://lkml.org/lkml/2008/4/14/38
# Fix:    commit e115f2d89253490fb2dbf304b627f8d908df26f1
#---------------------------------------------------------------------------
test_1()
{
	mkdir memcg/0/
	echo 0 > memcg/0/memory.limit_in_bytes

	./memcg_test_1

	rmdir memcg/0/

	check_kernel_bug
	if [ $? -eq 1 ]; then
		tst_resm TPASS "no kernel bug was found"
	fi
}

#---------------------------------------------------------------------------
# Bug:    Shrink memory might never return, unless send signal to stop it.
# Kernel: 2.6.29
# Links:  http://marc.info/?t=123199973900003&r=1&w=2
#         http://lkml.org/lkml/2009/2/3/72
# Fix:    81d39c20f5ee2437d71709beb82597e2a38efbbc
#---------------------------------------------------------------------------
test_2()
{
	./memcg_test_2 &
	pid1=$!
	sleep 1

	mkdir memcg/0
	echo $pid1 > memcg/0/tasks

	# let pid1 'test_2' allocate memory
	/bin/kill -SIGUSR1 $pid1
	sleep 1

	# shrink memory
	echo 1 > memcg/0/memory.limit_in_bytes 2>&1 &
	pid2=$!

	# check if 'echo' will exit and exit with failure
	for tmp in $(seq 0 4); do
		sleep 1
		ps -p $! > /dev/null
		if [ $? -ne 0 ]; then
			wait $pid2
			if [ $? -eq 0 ]; then
				tst_resm TFAIL "echo should return failure"
				failed=1
				kill -9 $pid1 $pid2 > /dev/null 2>&1
				wait $pid1 $pid2
				rmdir memcg/0
			fi
			break
		fi
	done

	if [ $tmp -eq 5 ]; then
		tst_resm TFAIL "'echo' doesn't exit!"
		failed=1
	else
		tst_resm TPASS "EBUSY was returned as expected"
	fi

	kill -9 $pid1 $pid2 > /dev/null 2>&1
	wait $pid1 $pid2 > /dev/null 2>&1
	rmdir memcg/0
}

#---------------------------------------------------------------------------
# Bug:    crash when rmdir a cgroup on IA64
# Kernel: 2.6.29-rcX
# Links:  http://marc.info/?t=123235660300001&r=1&w=2
# Fix:    commit 299b4eaa302138426d5a9ecd954de1f565d76c94
#---------------------------------------------------------------------------
test_3()
{
	mkdir memcg/0
	for pid in `cat memcg/tasks`; do
		echo $pid > memcg/0/tasks 2> /dev/null
	done

	for pid in `cat memcg/0/tasks`; do
		echo $pid > memcg/tasks 2> /dev/null
	done
	rmdir memcg/0

	check_kernel_bug
	if [ $? -eq 1 ]; then
		tst_resm TPASS "no kernel bug was found"
	fi
}

#---------------------------------------------------------------------------
# Bug:    the memcg's refcnt handling at swapoff was wrong, causing crash
# Kernel: 2.6.29-rcX
# Links:  http://marc.info/?t=123208656300004&r=1&w=2
# Fix:    commit 85d9fc89fb0f0703df6444f260187c088a8d59ff
#---------------------------------------------------------------------------
test_4()
{
	./memcg_test_4.sh

	check_kernel_bug
	if [ $? -eq 1 ]; then
		tst_resm TPASS "no kernel bug was found"
	fi

	# test_4.sh might be killed by oom, so do clean up here
	killall -9 memcg_test_4 2> /dev/null
	killall -9 memcg_test_4.sh 2> /dev/null

	# if test_4.sh gets killed, it won't clean cgroup it created
	rmdir memcg/0 2> /dev/null

	swapon -a
}

# main
failed=0
mkdir memcg/

for cur in $(seq 1 $TST_TOTAL); do
	export TST_COUNT=$cur

	mount -t cgroup -o memory xxx memcg/
	if [ $? -ne 0 ]; then
		tst_resm TFAIL "failed to mount memory subsystem"
		failed=1
		continue
	fi

	test_$cur

	umount memcg/
done

rmdir memcg/

exit $failed

