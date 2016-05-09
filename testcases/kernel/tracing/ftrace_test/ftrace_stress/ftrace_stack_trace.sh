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

LOOP=400

for ((; ;))
{
	for ((i = 0; i < $LOOP; i++))
	{
		cat "$TRACING_PATH"/stack_trace > /dev/null
	}

	sleep 1

	for ((i = 0; i < $LOOP; i++))
	{
		echo 0 > /proc/sys/kernel/stack_tracer_enabled
		echo 1 > /proc/sys/kernel/stack_tracer_enabled
	}

	sleep 1
}
