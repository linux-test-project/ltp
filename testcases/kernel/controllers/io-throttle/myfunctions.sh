#!/bin/sh
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
# usage . myfunctions.sh

setup()
{
	# create testcase cgroups
	if [ -e /dev/blockioctl ]; then
		echo "WARN: /dev/blockioctl already exist! overwriting."
		cleanup
	fi
	mkdir /dev/blockioctl
	mount -t cgroup -o blockio cgroup /dev/blockioctl
	if [ $? -ne 0 ]; then
		echo "ERROR: could not mount cgroup filesystem " \
			" on /dev/blockioctl. Exiting test."
		cleanup
		exit 1
	fi
	for i in `seq 1 3`; do
		if [ -e /dev/blockioctl/cgroup-$i ]; then
			rmdir /dev/blockioctl/cgroup-$i
			echo "WARN: earlier cgroup-$i found and removed"
		fi
		mkdir /dev/blockioctl/cgroup-$i
		if [ $? -ne 0 ]; then
			echo "ERROR: could not create cgroup-$i" \
				"Check your permissions. Exiting test."
			cleanup
			exit 1
		fi
	done
}

cleanup()
{
	echo "Cleanup called"
	for i in `seq 1 3`; do
		rmdir /dev/blockioctl/cgroup-$i
		rm -f /tmp/cgroup-$i.out
	done
	umount /dev/blockioctl
	rmdir /dev/blockioctl
}
