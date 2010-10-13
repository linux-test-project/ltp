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

LOOP=200

trace_options=(print-parent sym-offset sym-addr verbose raw hex bin block trace_printk ftrace_preempt branch annotate userstacktrace sym-userobj printk-msg-only context-info latency-format sleep-time graph-time)

NR_TRACE_OPTIONS=19

for ((; ; ))
{
	for ((j = 0; j < $LOOP; j++))
	{
		num=`date +%N`
		num=`printf 1%s $num`

		for ((i = 0; i < $NR_TRACE_OPTIONS; i++))
		{
			n=$(( ( $num >> $i ) % 2 ))
			if [ $n -eq 0 ]; then
				echo 0 > "$TRACING_PATH"/options/${trace_options[$i]}
			else
				echo 1 > "$TRACING_PATH"/options/${trace_options[$i]}
			fi
		}
	}

	sleep 1
}

