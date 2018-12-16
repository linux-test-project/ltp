#!/bin/sh
#
# Copyright (c) 2015 Fujitsu Ltd.
# Author: Zeng Linggang <zenglg.jy@cn.fujitsu.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# This is a regression test for commit:
# http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/commit/
# ?id=bb2bc55
#

TCID=cpuset_regression_test
TST_TOTAL=1
. test.sh

setup()
{
	tst_require_root

	if tst_kvcmp -lt "3.18"; then
		tst_brkm TCONF "Test must be run with kernel 3.18.0 or newer"
	fi

	local cpu_num=$(tst_getconf _NPROCESSORS_ONLN)
	if [ $cpu_num -lt 2 ]; then
		tst_brkm TCONF "We need 2 cpus at least to have test"
	fi

	tst_tmpdir

	TST_CLEANUP=cleanup

	# We need to mount cpuset if it is not found.
	mount_flag=0
	grep -w cpuset /proc/mounts > tmpfile
	if [ $? -eq 0 ]; then
		root_cpuset_dir=$(cat tmpfile | awk '{print $2}')
	else
		root_cpuset_dir="cpuset_test"

		ROD_SILENT mkdir -p ${root_cpuset_dir}

		ROD_SILENT mount -t cpuset cpuset ${root_cpuset_dir}

		mount_flag=1
	fi

	if [ -f ${root_cpuset_dir}/cpuset.cpu_exclusive ]; then
		cpu_exclusive=cpuset.cpu_exclusive
		cpus=cpuset.cpus
	elif [ -f ${root_cpuset_dir}/cpu_exclusive ]; then
		cpu_exclusive=cpu_exclusive
		cpus=cpus
	else
		tst_brkm TBROK "Both cpuset.cpu_exclusive and cpu_exclusive" \
			       "do not exist."
	fi

	cpu_exclusive_value=$(cat ${root_cpuset_dir}/${cpu_exclusive})
	if [ "${cpu_exclusive_value}" != "1" ];then
		echo 1 > ${root_cpuset_dir}/${cpu_exclusive}
		if [ $? -ne 0 ]; then
			tst_brkm TBROK "'echo 1 >" \
				       "${root_cpuset_dir}/${cpu_exclusive}'" \
				       "failed"
		fi
	fi
}

cleanup()
{
	if [ -d "${root_cpuset_dir}/testdir" ]; then
		rmdir ${root_cpuset_dir}/testdir
	fi

	if [ "$cpu_exclusive_value" != 1 ]; then
		# Need to flush, or may be output:
		# "write error: Device or resource busy"
		sync

		echo ${cpu_exclusive_value} > \
		     ${root_cpuset_dir}/${cpu_exclusive}
	fi

	if [ "${mount_flag}" = "1" ]; then
		umount ${root_cpuset_dir}
		if [ $? -ne 0 ]; then
			tst_resm TWARN "'umount ${root_cpuset_dir}' failed"
		fi

		if [ -d "${root_cpuset_dir}" ]; then
			rmdir ${root_cpuset_dir}
		fi
	fi

	tst_rmdir
}

cpuset_test()
{
	ROD_SILENT mkdir ${root_cpuset_dir}/testdir

	# Creat an exclusive cpuset.
	echo 1 > ${root_cpuset_dir}/testdir/${cpu_exclusive}
	if [ $? -ne 0 ]; then
		tst_brkm TFAIL "'echo 1 >" \
			       "${root_cpuset_dir}/testdir/${cpu_exclusive}'" \
			       "failed"
	fi

	local cpu_exclusive_tmp=$(cat \
				  ${root_cpuset_dir}/testdir/${cpu_exclusive})
	if [ "${cpu_exclusive_tmp}" != "1" ]; then
		tst_brkm TFAIL "${cpu_exclusive} is '${cpu_exclusive_tmp}'," \
			       "expected '1'"
	fi

	# ${cpus} is empty at the begin, that maybe make the system *crash*.
	echo 0-1 > ${root_cpuset_dir}/testdir/${cpus}
	if [ $? -ne 0 ]; then
		tst_brkm TFAIL "'echo 0-1 >" \
			       "${root_cpuset_dir}/testdir/${cpus}' failed"
	fi

	local cpus_value=$(cat ${root_cpuset_dir}/testdir/${cpus})
	if [ "${cpus_value}" != "0-1" ]; then
		tst_brkm TFAIL "${cpus} is '${cpus_value}', expected '0-1'"
	fi

	tst_resm TPASS "Bug is not reproduced"
}

setup

cpuset_test

tst_exit
