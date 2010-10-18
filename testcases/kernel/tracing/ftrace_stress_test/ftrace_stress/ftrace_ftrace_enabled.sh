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

MAX_LOOP=1500
count=0

for ((; ;))
{
	count=$(( $count + 1 ))

	for ((i = 0; i < $MAX_LOOP; i++))
	{
		echo 0 > /proc/sys/kernel/ftrace_enabled
		echo 1 > /proc/sys/kernel/ftrace_enabled
	}

	enable=$(( $count % 3 ))

	if [ $enable -eq 0 ]; then
		echo 1 > /proc/sys/kernel/ftrace_enabled
	else
		echo 0 > /proc/sys/kernel/ftrace_enabled
	fi

	sleep 1
}

