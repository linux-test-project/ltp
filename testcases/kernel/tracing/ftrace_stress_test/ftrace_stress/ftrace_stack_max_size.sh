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

MAX_STACK_SIZE=8192

if [ ! -e /proc/sys/kernel/stack_tracer_enabled ]; then
	should_skip=1
else
	should_skip=0
fi

for ((; ;))
{
	if [ $should_skip -eq 1 ]; then
		sleep 2
		continue
	fi

	for ((i = 0; i < $MAX_STACK_SIZE; i += 70))
	{
		echo $i > "$TRACING_PATH"/stack_max_size
		cat "$TRACING_PATH"/stack_max_size > /dev/null
	}

	sleep 1
}

