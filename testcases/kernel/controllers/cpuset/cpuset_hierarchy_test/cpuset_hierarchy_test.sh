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

export TCID="cpuset_hierarchy"
export TST_TOTAL=32
export TST_COUNT=1

. cpuset_funcs.sh

check

nr_cpus=$NR_CPUS
nr_mems=$N_NODES

cpus_all="$(seq -s, 0 $((nr_cpus-1)))"
mems_all="$(seq -s, 0 $((nr_mems-1)))"

exit_status=0

# test cpus

test1()
{
	echo > "$CPUSET/father/cpuset.cpus" || return 1
	echo > "$CPUSET/father/child/cpuset.cpus" || return 1

	cpuset_log "father cpuset.cpus $(cat "$CPUSET/father/cpuset.cpus")"
	cpuset_log "child cpuset.cpus $(cat "$CPUSET/father/child/cpuset.cpus")"

	test -z "$(cat "$CPUSET/father/cpuset.cpus")" || return 1
	test -z "$(cat "$CPUSET/father/child/cpuset.cpus")" || return 1
}

test2()
{
	echo > "$CPUSET/father/cpuset.cpus" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.cpus" 2> /dev/null && return 1

	cpuset_log "father cpuset.cpus $(cat "$CPUSET/father/cpuset.cpus")"
	cpuset_log "child cpuset.cpus $(cat "$CPUSET/father/child/cpuset.cpus")"

	test -z "$(cat "$CPUSET/father/cpuset.cpus")" || return 1
	test -z "$(cat "$CPUSET/father/child/cpuset.cpus")" || return 1
}

test3()
{
	echo 0 > "$CPUSET/father/cpuset.cpus" || return 1
	echo > "$CPUSET/father/child/cpuset.cpus" || return 1

	cpuset_log "father cpuset.cpus $(cat "$CPUSET/father/cpuset.cpus")"
	cpuset_log "child cpuset.cpus $(cat "$CPUSET/father/child/cpuset.cpus")"

	test 0 = "$(cat "$CPUSET/father/cpuset.cpus")" || return 1
	test -z "$(cat "$CPUSET/father/child/cpuset.cpus")" || return 1
}

test4()
{
	echo 0 > "$CPUSET/father/cpuset.cpus" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.cpus" || return 1

	cpuset_log "father cpuset.cpus $(cat "$CPUSET/father/cpuset.cpus")"
	cpuset_log "child cpuset.cpus $(cat "$CPUSET/father/child/cpuset.cpus")"

	test 0 = "$(cat "$CPUSET/father/cpuset.cpus")" || return 1
	test 0 = "$(cat "$CPUSET/father/child/cpuset.cpus")" || return 1
}

test5()
{
	echo 0 > "$CPUSET/father/cpuset.cpus" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.cpus" 2> /dev/null && return 1

	cpuset_log "father cpuset.cpus $(cat "$CPUSET/father/cpuset.cpus")"
	cpuset_log "child cpuset.cpus $(cat "$CPUSET/father/child/cpuset.cpus")"

	test 0 = "$(cat "$CPUSET/father/cpuset.cpus")" || return 1
	test -z "$(cat "$CPUSET/father/child/cpuset.cpus")" || return 1
}

test6()
{
	echo 0 > "$CPUSET/father/cpuset.cpus" || return 1
	echo 0,1 > "$CPUSET/father/child/cpuset.cpus" 2> /dev/null && return 1

	cpuset_log "father cpuset.cpus $(cat "$CPUSET/father/cpuset.cpus")"
	cpuset_log "child cpuset.cpus $(cat "$CPUSET/father/child/cpuset.cpus")"

	test 0 = "$(cat "$CPUSET/father/cpuset.cpus")" || return 1
	test -z "$(cat "$CPUSET/father/child/cpuset.cpus")" || return 1
}

test7()
{
	echo "0,1" > "$CPUSET/father/cpuset.cpus" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.cpus" || return 1

	cpuset_log "father cpuset.cpus $(cat "$CPUSET/father/cpuset.cpus")"
	cpuset_log "child cpuset.cpus $(cat "$CPUSET/father/child/cpuset.cpus")"

	test "0-1" = "$(cat "$CPUSET/father/cpuset.cpus")" || return 1
	test 0 = "$(cat "$CPUSET/father/child/cpuset.cpus")" || return 1
}

test8()
{
	echo "0,1" > "$CPUSET/father/cpuset.cpus" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.cpus" || return 1

	cpuset_log "father cpuset.cpus $(cat "$CPUSET/father/cpuset.cpus")"
	cpuset_log "child cpuset.cpus $(cat "$CPUSET/father/child/cpuset.cpus")"

	test "0-1" = "$(cat "$CPUSET/father/cpuset.cpus")" || return 1
	test 0 = "$(cat "$CPUSET/father/child/cpuset.cpus")" || return 1
}

test9()
{
	echo "$cpus_all" > "$CPUSET/father/cpuset.cpus" || return 1
	echo > "$CPUSET/father/child/cpuset.cpus" || return 1
	echo > "$CPUSET/father/cpuset.cpus" || return 1

	cpuset_log "father cpuset.cpus $(cat "$CPUSET/father/cpuset.cpus")"
	cpuset_log "child cpuset.cpus $(cat "$CPUSET/father/child/cpuset.cpus")"

	test -z "$(cat "$CPUSET/father/cpuset.cpus")" || return 1
	test -z "$(cat "$CPUSET/father/child/cpuset.cpus")" || return 1
}

test10()
{
	echo "$cpus_all" > "$CPUSET/father/cpuset.cpus" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.cpus" || return 1
	echo > "$CPUSET/father/cpuset.cpus" 2> /dev/null && return 1

	cpuset_log "father cpuset.cpus $(cat "$CPUSET/father/cpuset.cpus")"
	cpuset_log "child cpuset.cpus $(cat "$CPUSET/father/child/cpuset.cpus")"

	test "0-$((nr_cpus-1))" = "$(cat "$CPUSET/father/cpuset.cpus")" || return 1
	test 0 = "$(cat "$CPUSET/father/child/cpuset.cpus")" || return 1
}

test11()
{
	echo "$cpus_all" > "$CPUSET/father/cpuset.cpus" || return 1
	echo > "$CPUSET/father/child/cpuset.cpus" || return 1
	echo 0 > "$CPUSET/father/cpuset.cpus" || return 1

	cpuset_log "father cpuset.cpus $(cat "$CPUSET/father/cpuset.cpus")"
	cpuset_log "child cpuset.cpus $(cat "$CPUSET/father/child/cpuset.cpus")"

	test 0 = "$(cat "$CPUSET/father/cpuset.cpus")" || return 1
	test -z "$(cat "$CPUSET/father/child/cpuset.cpus")" || return 1
}

test12()
{
	echo "$cpus_all" > "$CPUSET/father/cpuset.cpus" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.cpus" || return 1
	echo 0 > "$CPUSET/father/cpuset.cpus" || return 1

	cpuset_log "father cpuset.cpus $(cat "$CPUSET/father/cpuset.cpus")"
	cpuset_log "child cpuset.cpus $(cat "$CPUSET/father/child/cpuset.cpus")"

	test 0 = "$(cat "$CPUSET/father/cpuset.cpus")" || return 1
	test 0 = "$(cat "$CPUSET/father/child/cpuset.cpus")" || return 1
}

test13()
{
	echo "$cpus_all" > "$CPUSET/father/cpuset.cpus" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.cpus" || return 1
	echo 0 > "$CPUSET/father/cpuset.cpus" 2> /dev/null && return 1

	cpuset_log "father cpuset.cpus $(cat "$CPUSET/father/cpuset.cpus")"
	cpuset_log "child cpuset.cpus $(cat "$CPUSET/father/child/cpuset.cpus")"

	test "0-$((nr_cpus-1))" = "$(cat "$CPUSET/father/cpuset.cpus")" || return 1
	test 1 = "$(cat "$CPUSET/father/child/cpuset.cpus")" || return 1
}

test14()
{
	echo "$cpus_all" > "$CPUSET/father/cpuset.cpus" || return 1
	echo 0,1 > "$CPUSET/father/child/cpuset.cpus" || return 1
	echo 0 > "$CPUSET/father/cpuset.cpus" 2> /dev/null && return 1

	cpuset_log "father cpuset.cpus $(cat "$CPUSET/father/cpuset.cpus")"
	cpuset_log "child cpuset.cpus $(cat "$CPUSET/father/child/cpuset.cpus")"

	test "0-$((nr_cpus-1))" = "$(cat "$CPUSET/father/cpuset.cpus")" || return 1
	test "0-1" = "$(cat "$CPUSET/father/child/cpuset.cpus")" || return 1
}

test15()
{
	echo "$cpus_all" > "$CPUSET/father/cpuset.cpus" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.cpus" || return 1
	echo "0,1" > "$CPUSET/father/cpuset.cpus" || return 1

	cpuset_log "father cpuset.cpus $(cat "$CPUSET/father/cpuset.cpus")"
	cpuset_log "child cpuset.cpus $(cat "$CPUSET/father/child/cpuset.cpus")"

	test "0-1" = "$(cat "$CPUSET/father/cpuset.cpus")" || return 1
	test 0 = "$(cat "$CPUSET/father/child/cpuset.cpus")" || return 1
}

test16()
{
	echo "$cpus_all" > "$CPUSET/father/cpuset.cpus" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.cpus" || return 1
	echo "0,1" > "$CPUSET/father/cpuset.cpus" || return 1

	cpuset_log "father cpuset.cpus $(cat "$CPUSET/father/cpuset.cpus")"
	cpuset_log "child cpuset.cpus $(cat "$CPUSET/father/child/cpuset.cpus")"

	test "0-1" = "$(cat "$CPUSET/father/cpuset.cpus")" || return 1
	test 0 = "$(cat "$CPUSET/father/child/cpuset.cpus")" || return 1
}

## test mems

test17()
{
	echo > "$CPUSET/father/cpuset.mems" || return 1
	echo > "$CPUSET/father/child/cpuset.mems" || return 1

	cpuset_log "father cpuset.mems $(cat "$CPUSET/father/cpuset.mems")"
	cpuset_log "child cpuset.mems $(cat "$CPUSET/father/child/cpuset.mems")"

	test -z "$(cat "$CPUSET/father/cpuset.mems")" || return 1
	test -z "$(cat "$CPUSET/father/child/cpuset.mems")" || return 1
}

test18()
{
	echo > "$CPUSET/father/cpuset.mems" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.mems" 2> /dev/null && return 1

	cpuset_log "father cpuset.mems $(cat "$CPUSET/father/cpuset.mems")"
	cpuset_log "child cpuset.mems $(cat "$CPUSET/father/child/cpuset.mems")"

	test -z "$(cat "$CPUSET/father/cpuset.mems")" || return 1
	test -z "$(cat "$CPUSET/father/child/cpuset.mems")" || return 1
}

test19()
{
	echo 0 > "$CPUSET/father/cpuset.mems" || return 1
	echo > "$CPUSET/father/child/cpuset.mems" || return 1

	cpuset_log "father cpuset.mems $(cat "$CPUSET/father/cpuset.mems")"
	cpuset_log "child cpuset.mems $(cat "$CPUSET/father/child/cpuset.mems")"

	test 0 = "$(cat "$CPUSET/father/cpuset.mems")" || return 1
	test -z "$(cat "$CPUSET/father/child/cpuset.mems")" || return 1
}

test20()
{
	echo 0 > "$CPUSET/father/cpuset.mems" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.mems" || return 1

	cpuset_log "father cpuset.mems $(cat "$CPUSET/father/cpuset.mems")"
	cpuset_log "child cpuset.mems $(cat "$CPUSET/father/child/cpuset.mems")"

	test 0 = "$(cat "$CPUSET/father/cpuset.mems")" || return 1
	test 0 = "$(cat "$CPUSET/father/child/cpuset.mems")" || return 1
}

test21()
{
	echo 0 > "$CPUSET/father/cpuset.mems" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.mems" 2> /dev/null && return 1

	cpuset_log "father cpuset.mems $(cat "$CPUSET/father/cpuset.mems")"
	cpuset_log "child cpuset.mems $(cat "$CPUSET/father/child/cpuset.mems")"

	test 0 = "$(cat "$CPUSET/father/cpuset.mems")" || return 1
	test -z "$(cat "$CPUSET/father/child/cpuset.mems")" || return 1
}

test22()
{
	echo 0 > "$CPUSET/father/cpuset.mems" || return 1
	echo 0,1 > "$CPUSET/father/child/cpuset.mems" 2> /dev/null && return 1

	cpuset_log "father cpuset.mems $(cat "$CPUSET/father/cpuset.mems")"
	cpuset_log "child cpuset.mems $(cat "$CPUSET/father/child/cpuset.mems")"

	test 0 = "$(cat "$CPUSET/father/cpuset.mems")" || return 1
	test -z "$(cat "$CPUSET/father/child/cpuset.mems")" || return 1
}

test23()
{
	echo "0,1" > "$CPUSET/father/cpuset.mems" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.mems" || return 1

	cpuset_log "father cpuset.mems $(cat "$CPUSET/father/cpuset.mems")"
	cpuset_log "child cpuset.mems $(cat "$CPUSET/father/child/cpuset.mems")"

	test "0-1" = "$(cat "$CPUSET/father/cpuset.mems")" || return 1
	test 0 = "$(cat "$CPUSET/father/child/cpuset.mems")" || return 1
}

test24()
{
	echo "0,1" > "$CPUSET/father/cpuset.mems" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.mems" || return 1

	cpuset_log "father cpuset.mems $(cat "$CPUSET/father/cpuset.mems")"
	cpuset_log "child cpuset.mems $(cat "$CPUSET/father/child/cpuset.mems")"

	test "0-1" = "$(cat "$CPUSET/father/cpuset.mems")" || return 1
	test 0 = "$(cat "$CPUSET/father/child/cpuset.mems")" || return 1
}

test25()
{
	echo "$mems_all" > "$CPUSET/father/cpuset.mems" || return 1
	echo > "$CPUSET/father/child/cpuset.mems" || return 1
	echo > "$CPUSET/father/cpuset.mems" || return 1

	cpuset_log "father cpuset.mems $(cat "$CPUSET/father/cpuset.mems")"
	cpuset_log "child cpuset.mems $(cat "$CPUSET/father/child/cpuset.mems")"

	test -z "$(cat "$CPUSET/father/cpuset.mems")" || return 1
	test -z "$(cat "$CPUSET/father/child/cpuset.mems")" || return 1
}

test26()
{
	echo "$mems_all" > "$CPUSET/father/cpuset.mems" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.mems" || return 1
	echo > "$CPUSET/father/cpuset.mems" 2> /dev/null && return 1

	cpuset_log "father cpuset.mems $(cat "$CPUSET/father/cpuset.mems")"
	cpuset_log "child cpuset.mems $(cat "$CPUSET/father/child/cpuset.mems")"

	test "0-$((nr_mems-1))" = "$(cat "$CPUSET/father/cpuset.mems")" || return 1
	test 0 = "$(cat "$CPUSET/father/child/cpuset.mems")" || return 1
}

test27()
{
	echo "$mems_all" > "$CPUSET/father/cpuset.mems" || return 1
	echo > "$CPUSET/father/child/cpuset.mems" || return 1
	echo 0 > "$CPUSET/father/cpuset.mems" || return 1

	cpuset_log "father cpuset.mems $(cat "$CPUSET/father/cpuset.mems")"
	cpuset_log "child cpuset.mems $(cat "$CPUSET/father/child/cpuset.mems")"

	test 0 = "$(cat "$CPUSET/father/cpuset.mems")" || return 1
	test -z "$(cat "$CPUSET/father/child/cpuset.mems")" || return 1
}

test28()
{
	echo "$mems_all" > "$CPUSET/father/cpuset.mems" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.mems" || return 1
	echo 0 > "$CPUSET/father/cpuset.mems" || return 1

	cpuset_log "father cpuset.mems $(cat "$CPUSET/father/cpuset.mems")"
	cpuset_log "child cpuset.mems $(cat "$CPUSET/father/child/cpuset.mems")"

	test 0 = "$(cat "$CPUSET/father/cpuset.mems")" || return 1
	test 0 = "$(cat "$CPUSET/father/child/cpuset.mems")" || return 1
}

test29()
{
	echo "$mems_all" > "$CPUSET/father/cpuset.mems" || return 1
	echo 1 > "$CPUSET/father/child/cpuset.mems" || return 1
	echo 0 > "$CPUSET/father/cpuset.mems" 2> /dev/null && return 1

	cpuset_log "father cpuset.mems $(cat "$CPUSET/father/cpuset.mems")"
	cpuset_log "child cpuset.mems $(cat "$CPUSET/father/child/cpuset.mems")"

	test "0-$((nr_mems-1))" = "$(cat "$CPUSET/father/cpuset.mems")" || return 1
	test 1 = "$(cat "$CPUSET/father/child/cpuset.mems")" || return 1
}

test30()
{
	echo "$mems_all" > "$CPUSET/father/cpuset.mems" || return 1
	echo 0,1 > "$CPUSET/father/child/cpuset.mems" || return 1
	echo 0 > "$CPUSET/father/cpuset.mems" 2> /dev/null && return 1

	cpuset_log "father cpuset.mems $(cat "$CPUSET/father/cpuset.mems")"
	cpuset_log "child cpuset.mems $(cat "$CPUSET/father/child/cpuset.mems")"

	test "0-$((nr_mems-1))" = "$(cat "$CPUSET/father/cpuset.mems")" || return 1
	test "0-1" = "$(cat "$CPUSET/father/child/cpuset.mems")" || return 1
}

test31()
{
	echo "$mems_all" > "$CPUSET/father/cpuset.mems" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.mems" || return 1
	echo "0,1" > "$CPUSET/father/cpuset.mems" || return 1

	cpuset_log "father cpuset.mems $(cat "$CPUSET/father/cpuset.mems")"
	cpuset_log "child cpuset.mems $(cat "$CPUSET/father/child/cpuset.mems")"

	test "0-1" = "$(cat "$CPUSET/father/cpuset.mems")" || return 1
	test 0 = "$(cat "$CPUSET/father/child/cpuset.mems")" || return 1
}

test32()
{
	echo "$mems_all" > "$CPUSET/father/cpuset.mems" || return 1
	echo 0 > "$CPUSET/father/child/cpuset.mems" || return 1
	echo "0,1" > "$CPUSET/father/cpuset.mems" || return 1

	cpuset_log "father cpuset.mems $(cat "$CPUSET/father/cpuset.mems")"
	cpuset_log "child cpuset.mems $(cat "$CPUSET/father/child/cpuset.mems")"

	test "0-1" = "$(cat "$CPUSET/father/cpuset.mems")" || return 1
	test 0 = "$(cat "$CPUSET/father/child/cpuset.mems")" || return 1
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
			tst_resm TFAIL "Break the hierarchy limit."
			exit_status=1
		else
			tst_resm TPASS "Hierarchy test succeeded."
		fi

		cleanup
		if [ $? -ne 0 ]; then
			exit_status=1
		fi
	fi
	TST_COUNT=$(($TST_COUNT + 1))
done

exit $exit_status
