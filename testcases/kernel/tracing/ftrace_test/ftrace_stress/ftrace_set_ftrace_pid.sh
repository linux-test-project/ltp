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

LOOP=300

if [ ! -e "$TRACING_PATH"/set_ftrace_pid ]; then
	should_skip=1
else
	should_skip=0
fi

for ((; ; ))
{
	if [ $should_skip -eq 1 ]; then
		sleep 2
		continue
	fi

	for ((j = 0; j < $LOOP; j++))
	{
		for ((k = 1; k <= NR_PIDS; k++))
		{
			str="\$pid$k"
			eval echo $str >> "$TRACING_PATH"/set_ftrace_pid
		}

		if ! echo > "$TRACING_PATH"/set_ftrace_pid >/dev/null 2>&1; then
			if ! echo -1 > "$TRACING_PATH"/set_ftrace_pid >/dev/null 2>&1; then
				tst_resm TBROK "Cannot disable set_ftrace_pid!"
				exit 1
			fi
		fi
	}

	sleep 1
}

