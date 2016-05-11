#! /bin/sh

###############################################################################
#                                                                             #
# Copyright (c) 2010 FUJITSU LIMITED                                          #
#                                                                             #
# This program is free software; you can redistribute it and/or modify it     #
# under the terms of the GNU General Public License as published by the Free  #
# Software Foundation; either version 2 of the License, or (at your option)   #
# any later version.                                                          #
#                                                                             #
# Author: Li Zefan <lizf@cn.fujitsu.com>                                      #
#                                                                             #
###############################################################################

. test.sh

LOOP=200

while true; do
	j=0
	while [ $j -lt $LOOP ]; do
		trace_options="$(ls $TRACING_PATH/options/)"
		# enable the nop_test_refuse can cause an
		# 'write error: Invalid argument'. So don't test it.
		trace_options="$(echo $trace_options | sed 's/test_nop_refuse//')"
		nr_trace_options=$(echo "${trace_options}" | wc -w)

		option_index=$(tst_random 1 $nr_trace_options)
		option=$(echo "$trace_options" | awk "{print \$$option_index}")
		i=0
		while [ $i -lt $nr_trace_options ]; do
			n=$(tst_random 0 1)
			opt_f="$TRACING_PATH"/options/$option
			ret_val=0
			if [ $n -eq 0 ]; then
				operation="setup"
			else
				operation="clear"
			fi
			# On old kernel, some trace option dirs
			# won't be made if the option has nothing
			# to do with the current tracer. But on newer
			# kernel(4.4-rc1), all option dirs will be made.
			# So here check it to avoid 'Permision denied'
			if [ -f $opt_f ]; then
				echo $n > $opt_f
				ret_val=$?
			fi

			if [ $ret_val -ne 0 ]; then
				tst_resm TFAIL "$0: $operation trace option $option failed"
			fi
			i=$((i + 1))
		done
		j=$((j + 1))
	done
done
