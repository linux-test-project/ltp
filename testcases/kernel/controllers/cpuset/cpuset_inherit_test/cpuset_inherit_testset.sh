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

export TCID="cpuset_inherit"
export TST_TOTAL=33
export TST_COUNT=1

. cpuset_funcs.sh

check 1 1

nr_cpus=$NR_CPUS
nr_mems=$N_NODES

cpus_all="$(seq -s, 0 $((nr_cpus-1)))"
mems_all="$(seq -s, 0 $((nr_mems-1)))"

exit_status=0

cfile_name=

# base_op_write_and_test <write_file_name> <write_string> <expect_string>
base_op_write_and_test()
{
	local write_file="$1"
	local inherit_value="$2"
	local write_string="$3"
	local expect_string="$4"
	local return_result=

	echo $inherit_value > $CLONE_CHILDREN

	mkdir -p "$(dirname $write_file)" || {
		tst_brkm TFAIL "Failed to mkdir -p $(basename $write_file)"
		return 1
	}
	[ "$write_string" = NULL ] && write_string=" "

	/bin/echo "$write_string" > "$write_file" 2> $CPUSET_TMP/stderr
	mkdir $(dirname $write_file)/2 2> $CPUSET_TMP/stderr
	return_result=$?
	write_result="$(cat "$(dirname $write_file)/2/$(basename $write_file)")"

	case "$expect_string" in
	EMPTY)
		test -z "$write_result" -a $return_result = 0
		return_result=$?
		;;
	WRITE_ERROR)
		return_result=$((!$return_result))
		;;
	*)
		test "$expect_string" = "$write_result" -a $return_result = 0
		return_result=$?
		;;
	esac

	if [ $return_result -eq 0 ]; then
		tst_resm TPASS "$cfile_name: Inherited information is right!"
        else
		tst_resm TFAIL "$cfile_name: Test result - $write_result Expected string - \"$expect_string\""
        fi
        return $return_result
}

inherit_test()
{
	setup
	if [ $? -ne 0 ]; then
		exit_status=1
	else
		base_op_write_and_test "$@"
		if [ $? -ne 0 ]; then
			exit_status=1
		fi

		cleanup
		if [ $? -ne 0 ]; then
			exit_status=1
		fi
	fi
	TST_COUNT=$(($TST_COUNT + 1))
}

test_cpus()
{
	cfile_name="cpus"
	num=$((nr_cpus-1))
	cpu_string="0-$num"
	if [ $nr_cpus -eq 1 ]; then
		cpu_string="0"
	fi
	while read inherit cpus result
	do
		inherit_test "$CPUSET/1/cpuset.cpus" "$inherit" "$cpus" "$result"
	done <<- EOF
		0	NULL					EMPTY
		0	0					EMPTY
		0	$cpus_all				EMPTY
		1	NULL					EMPTY
		1	0					0
		1	$cpus_all				$cpu_string
	EOF
	# while read cpus result
}

test_mems()
{
	cfile_name="mems"
	while read inherit mems result
	do
		inherit_test "$CPUSET/1/cpuset.mems" "$inherit" "$mems" "$result"
	done <<- EOF
		0	NULL					EMPTY
		0	0					EMPTY
		0	$mems_all				EMPTY
		1	NULL					EMPTY
		1	0					0
		1	$mems_all				$mem_string
	EOF
	# while read mems result
}

# test cpu_exclusive mem_exclusive mem_hardwall memory_migrate
test_three_result_similar_flags()
{
	for filename in cpu_exclusive mem_exclusive mem_hardwall \
			memory_migrate
	do
		cfile_name="$filename"
		while read inherit flags result
		do
			inherit_test "$CPUSET/1/cpuset.$filename" "$inherit" "$flags" "$result"
		done <<- EOF
			0	0	0
			0	1	0
		EOF
		# while read flags, result
	done # for filename in flagfiles
}

# test memory_spread_page memory_spread_slab
test_spread_flags()
{
	for filename in memory_spread_page memory_spread_slab
	do
		cfile_name="$filename"
		while read inherit flags result
		do
			inherit_test "$CPUSET/1/cpuset.$filename" "$inherit" "$flags" "$result"
		done <<- EOF
			0	0	0
			0	1	1
		EOF
		# while read flags, result
	done # for filename in flagfiles
}

test_sched_load_balance_flag()
{
	cfile_name="sched_load_balance"
	while read inherit flag result
	do
		inherit_test "$CPUSET/1/cpuset.sched_load_balance" "$inherit" "$flag" "$result"
	done <<- EOF
		0	0	1
		0	1	1
	EOF
	# while read mems result
}

test_domain()
{
	cfile_name="sched_relax_domain_level"
	while read inherit domain_level result
	do
		inherit_test "$CPUSET/1/cpuset.sched_relax_domain_level" "$inherit" "$domain_level" "$result"
	done <<- EOF
		0	-1	-1
		0	0	-1
		0	1	-1
		0	2	-1
		0	3	-1
		0	4	-1
		0	5	-1
	EOF
	# while read domain_level result
}

# Case 1-6
test_cpus

# Case 7-12
test_mems

# Case 13-20
test_three_result_similar_flags

# Case 21-24
test_spread_flags

# Case 25-26
test_sched_load_balance_flag

# Case 27-33
test_domain

exit $exit_status
