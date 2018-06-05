#!/bin/sh

################################################################################
#                                                                              #
# Copyright (c) 2009 FUJITSU LIMITED                                           #
#                                                                              #
# This program is free software;  you can redistribute it and#or modify        #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; either version 2 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# This program is distributed in the hope that it will be useful, but          #
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY   #
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License     #
# for more details.                                                            #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program;  if not, write to the Free Software                 #
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA      #
#                                                                              #
# Author: Miao Xie <miaox@cn.fujitsu.com>                                      #
#                                                                              #
################################################################################

export TCID="cpuset_exclusive"
export TST_TOTAL=18
export TST_COUNT=1

. cpuset_funcs.sh

check

exit_status=0

# Case 1-9 test cpus
test1()
{
	echo 0 > "$CPUSET/father/cpuset.cpu_exclusive" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.cpu_exclusive" || return 1

	test 0 = $(cat "$CPUSET/father/child/cpuset.cpu_exclusive") || return 1
}

test2()
{
	echo 1 > "$CPUSET/father/cpuset.cpu_exclusive" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.cpu_exclusive" || return 1

	test 0 = $(cat "$CPUSET/father/child/cpuset.cpu_exclusive") || return 1
}

test3()
{
	echo 1 > "$CPUSET/father/cpuset.cpu_exclusive" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.cpu_exclusive" || return 1

	test 1 = $(cat "$CPUSET/father/child/cpuset.cpu_exclusive") || return 1
}

test4()
{
	echo 0 > "$CPUSET/father/cpuset.cpu_exclusive" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.cpu_exclusive" 2> /dev/null && return 1

	test 0 = $(cat "$CPUSET/father/child/cpuset.cpu_exclusive") || return 1
}

test5()
{
	echo 1 > "$CPUSET/father/cpuset.cpu_exclusive" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.cpu_exclusive" || return 1
	echo 0 > "$CPUSET/father/cpuset.cpu_exclusive" 2> /dev/null && return 1

	test 1 = $(cat "$CPUSET/father/cpuset.cpu_exclusive") || return 1
}

test6()
{
	echo 1 > "$CPUSET/father/cpuset.cpu_exclusive" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.cpu_exclusive" || return 1
	echo "0-1" > "$CPUSET/father/cpuset.cpus" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.cpus" || return 1
	mkdir "$CPUSET/father/other" || return 1
	echo 1 > "$CPUSET/father/other/cpuset.cpus" || return 1

	test 0 = $(cat "$CPUSET/father/child/cpuset.cpus") || return 1
	test 1 = $(cat "$CPUSET/father/other/cpuset.cpus") || return 1
}

test7()
{
	echo 1 > "$CPUSET/father/cpuset.cpu_exclusive" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.cpu_exclusive" || return 1
	echo "0-1" > "$CPUSET/father/cpuset.cpus" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.cpus" || return 1
	mkdir "$CPUSET/father/other" || return 1
	echo "0-1" > "$CPUSET/father/other/cpuset.cpus" 2> /dev/null && return 1

	test 0 = $(cat "$CPUSET/father/child/cpuset.cpus") || return 1
	test -z $(cat "$CPUSET/father/other/cpuset.cpus") || return 1
}

test8()
{
	echo 1 > "$CPUSET/father/cpuset.cpu_exclusive" || return 1
	echo "0-1" > "$CPUSET/father/cpuset.cpus" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.cpus" || return 1
	mkdir "$CPUSET/father/other" || return 1
	echo 1 > "$CPUSET/father/other/cpuset.cpus" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.cpu_exclusive" || return 1

	test 1 = $(cat "$CPUSET/father/child/cpuset.cpu_exclusive") || return 1
}

test9()
{
	echo 1 > "$CPUSET/father/cpuset.cpu_exclusive" || return 1
	echo "0-1" > "$CPUSET/father/cpuset.cpus" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.cpus" || return 1
	mkdir "$CPUSET/father/other" || return 1
	echo "0-1" > "$CPUSET/father/other/cpuset.cpus" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.cpu_exclusive" 2> /dev/null && return 1

	test 0 = $(cat "$CPUSET/father/child/cpuset.cpu_exclusive") || return 1
}

# The following cases test mems

test10()
{
	echo 0 > "$CPUSET/father/cpuset.mem_exclusive" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.mem_exclusive" || return 1

	test 0 = $(cat "$CPUSET/father/child/cpuset.mem_exclusive") || return 1
}

test11()
{
	echo 1 > "$CPUSET/father/cpuset.mem_exclusive" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.mem_exclusive" || return 1

	test 0 = $(cat "$CPUSET/father/child/cpuset.mem_exclusive") || return 1
}

test12()
{
	echo 1 > "$CPUSET/father/cpuset.mem_exclusive" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.mem_exclusive" || return 1

	test 1 = $(cat "$CPUSET/father/child/cpuset.mem_exclusive") || return 1
}

test13()
{
	echo 0 > "$CPUSET/father/cpuset.mem_exclusive" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.mem_exclusive" 2> /dev/null && return 1

	test 0 = $(cat "$CPUSET/father/child/cpuset.mem_exclusive") || return 1
}

test14()
{
	echo 1 > "$CPUSET/father/cpuset.mem_exclusive" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.mem_exclusive" || return 1
	echo 0 > "$CPUSET/father/cpuset.mem_exclusive" 2> /dev/null && return 1

	test 1 = $(cat "$CPUSET/father/cpuset.mem_exclusive") || return 1
}

test15()
{
	echo 1 > "$CPUSET/father/cpuset.mem_exclusive" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.mem_exclusive" || return 1
	echo "0-1" > "$CPUSET/father/cpuset.mems" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.mems" || return 1
	mkdir "$CPUSET/father/other" || return 1
	echo 1 > "$CPUSET/father/other/cpuset.mems" || return 1

	test 0 = $(cat "$CPUSET/father/child/cpuset.mems") || return 1
	test 1 = $(cat "$CPUSET/father/other/cpuset.mems") || return 1
}

test16()
{
	echo 1 > "$CPUSET/father/cpuset.mem_exclusive" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.mem_exclusive" || return 1
	echo "0-1" > "$CPUSET/father/cpuset.mems" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.mems" || return 1
	mkdir "$CPUSET/father/other" || return 1
	echo "0-1" > "$CPUSET/father/other/cpuset.mems" 2> /dev/null && return 1

	test 0 = $(cat "$CPUSET/father/child/cpuset.mems") || return 1
	test -z $(cat "$CPUSET/father/other/cpuset.mems") || return 1
}

test17()
{
	echo 1 > "$CPUSET/father/cpuset.mem_exclusive" || return 1
	echo "0-1" > "$CPUSET/father/cpuset.mems" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.mems" || return 1
	mkdir "$CPUSET/father/other" || return 1
	echo 1 > "$CPUSET/father/other/cpuset.mems" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.mem_exclusive" || return 1

	test 1 = $(cat "$CPUSET/father/child/cpuset.mem_exclusive") || return 1
}

test18()
{
	echo 1 > "$CPUSET/father/cpuset.mem_exclusive" || return 1
	echo "0-1" > "$CPUSET/father/cpuset.mems" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.mems" || return 1
	mkdir "$CPUSET/father/other" || return 1
	echo "0-1" > "$CPUSET/father/other/cpuset.mems" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.mem_exclusive" 2> /dev/null && return 1

	test 0 = $(cat "$CPUSET/father/child/cpuset.mem_exclusive") || return 1
}

for i in $(seq 1 $TST_TOTAL)
do
	setup
	if [ $? -ne 0 ]; then
		exit_status=1
	else
		mkdir "$CPUSET/father"
		mkdir "$CPUSET/father/child"
		test$i
		if [ $? -ne 0 ]; then
			tst_resm TFAIL "Break the exclusive feature."
			exit_status=1
		else
			tst_resm TPASS "Exclusive test succeeded."
		fi

		cleanup
		if [ $? -ne 0 ]; then
			exit_status=1
		fi
	fi
	TST_COUNT=$(($TST_COUNT + 1))
done

exit $exit_status
