#! /bin/sh

###########################################################################
##                                                                       ##
## Copyright (c) 2015, Red Hat Inc.                                      ##
##                                                                       ##
## This program is free software: you can redistribute it and/or modify  ##
## it under the terms of the GNU General Public License as published by  ##
## the Free Software Foundation, either version 3 of the License, or     ##
## (at your option) any later version.                                   ##
##                                                                       ##
## This program is distributed in the hope that it will be useful,       ##
## but WITHOUT ANY WARRANTY; without even the implied warranty of        ##
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          ##
## GNU General Public License for more details.                          ##
##                                                                       ##
## You should have received a copy of the GNU General Public License     ##
## along with this program. If not, see <http://www.gnu.org/licenses/>.  ##
##                                                                       ##
## Author: Chunyu Hu <chuhu@redhat.com>                                  ##
##                                                                       ##
###########################################################################

. test.sh
nr_cpus=`tst_ncpus`

# the 32 bit integer count 32 cpus. One integer is not
# enough to store the cpu mask for nr_cpu > 32.
if [ $nr_cpus -gt 32 ]; then
	group_cnt=$((nr_cpus / 32))
	range=31
	rem=$((nr_cpus % 32))
	if [ $rem -ne 0 ]; then
		range_last=$((rem -1))
	fi
else
	group_cnt=1
	range=$((nr_cpus - 1))
fi

get_test_cpumask()
{
	mask=""

	local i=0
	while [ $i -lt $group_cnt ]; do
		# select count of cpu in one group, include the duplicate.
		local set_cnt=$(tst_random 1 $((range + 1)))

		local c=0
		local temp_mask=0
		while [ $c -lt $set_cnt ]; do
			local group_cpuid=$(tst_random 1 $range)
			temp_mask=$((temp_mask | $((1 << $group_cpuid))))
			c=$((c + 1))
		done

		if  [ $i = 0 ]; then
			mask=`echo $temp_mask | awk '{printf "%x",$0}'`
		else
			mask=$mask","`echo $temp_mask | awk '{printf "%x",$0}'`
		fi

		i=$((i + 1))
	done

	if [ $group_cnt -gt 1  ]; then
		set_cnt=$(tst_random 1 $((range_last +1)))
		c=0;
		temp_mask=0
		while [ $c -lt $set_cnt ]; do
			local group_cpuid=$(tst_random 1 $range_last)
			temp_mask=$((temp_mask | $((1 << $group_cpuid))))
			c=$((c + 1))
		done
		mask=`echo $temp_mask | awk '{printf "%x",$0}'`
	fi

	echo "$mask"
}

signal_handler()
{
	tst_exit
}

trap signal_handler SIGTERM SIGKILL

while true; do
	get_test_cpumask > $TRACING_PATH/tracing_cpumask
done
