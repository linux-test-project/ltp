#!/bin/bash
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 021110-1307, USA.
#
# Copyright (C) 2008 Andrea Righi <righi.andrea@gmail.com>
#
# Usage: ./run_io_throttle_test.sh
# Description: test block device I/O bandwidth controller functionalities

. ./myfunctions-io.sh

trap cleanup SIGINT

BUFSIZE=16m
DATASIZE=64m

setup

# get the device name of the entire mounted block device
dev=`df -P . | sed '1d' | cut -d' ' -f1 | sed 's/[p]*[0-9]*$//'`

# evaluate device bandwidth
export MYGROUP=
phys_bw=`./iobw -direct 1 $BUFSIZE $DATASIZE | grep TOTAL | awk '{print $7}'`
if [ $? -ne 0 ]; then
	echo "ERROR: could not evaluate i/o bandwidth of $dev. Exiting test."
	cleanup
	exit 1
fi
echo ">> physical i/o bandwidth limit is: $phys_bw KiB/s"
# show cgroup i/o bandwidth limits
for i in `seq 1 3`; do
	MYGROUP=cgroup-$i
	echo "($MYGROUP) max i/o bw: " \
		"$(($phys_bw / `echo 2^$i | bc`)) KiB/s + O_DIRECT"
done

for tasks in 1 2 4; do
for strategy in 0 1; do
	# set bw limiting rules
	if [ -f /dev/blockioctl/blockio.bandwidth ]; then
		io_throttle_file=blockio.bandwidth
	elif [ -f /dev/blockioctl/blockio.bandwidth-max ]; then
		io_throttle_file=blockio.bandwidth-max
	else
		echo "ERROR: unknown kernel ABI. Exiting test."
		cleanup
		exit 1
	fi
	for i in `seq 1 3`; do
		limit=$(($phys_bw * 1024 / `echo 2^$i | bc`))
		IOBW[$i]=$(($limit / 1024))
		/bin/echo $dev:$limit:$strategy:$limit > \
			/dev/blockioctl/cgroup-$i/${io_throttle_file}
		if [ $? -ne 0 ]; then
			echo "ERROR: could not set i/o bandwidth limit for cgroup-$i. Exiting test."
			cleanup
			exit 1
		fi
	done

	# run benchmark
	if [ $tasks -eq 1 ]; then
		stream="stream"
	else
		stream="streams"
	fi
	echo -n ">> testing $tasks parallel $stream per cgroup "
	if [ $strategy -eq 0 ]; then
		echo "(leaky-bucket i/o throttling)"
	else
		echo "(token-bucket i/o throttling)"
	fi
	for i in `seq 1 3`; do
		MYGROUP=cgroup-$i
		/bin/echo $$ > /dev/blockioctl/$MYGROUP/tasks
		if [ $? -ne 0 ]; then
			echo "ERROR: could not set i/o bandwidth limit for cgroup-$i. Exiting test."
			cleanup
			exit 1
		fi
		# exec i/o benchmark
		./iobw -direct $tasks $BUFSIZE $DATASIZE > /tmp/$MYGROUP.out &
		PID[$i]=$!
	done
	/bin/echo $$ > /dev/blockioctl/tasks

	# wait for children completion
	for i in `seq 1 3`; do
		MYGROUP=cgroup-$i
		wait ${PID[$i]}
		ret=$?
		if [ $ret -ne 0 ]; then
			echo "ERROR: error code $ret during test $tasks.$strategy.$i. Exiting test."
			cleanup
			exit 1
		fi
		iorate=`grep parent /tmp/${MYGROUP}.out | awk '{print $7}'`
		diff=$((${IOBW[$i]} - $iorate))
		echo "($MYGROUP) i/o-bw ${IOBW[$i]} KiB/s, i/o-rate $iorate KiB/s, err $diff KiB/s"
		if [ ${IOBW[$i]} -ge $iorate ]; then
			echo "TPASS   Block device I/O bandwidth controller: test $tasks.$strategy.$i PASSED";
		else
			echo "TFAIL   Block device I/O bandwidth controller: test $tasks.$strategy.$i FAILED";
		fi
	done
done
done

cleanup
