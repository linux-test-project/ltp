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

for ((; ;))
{
	for ((i = 0; i < 100; i++))
	{
		echo 1 > "$TRACING_PATH"/events/enable
		echo 0 > "$TRACING_PATH"/events/enable
	}

	for dir in `ls $TRACING_PATH/events/`
	do
		if [ ! -d $dir -o "$dir" = ftrace ]; then
			continue;
		fi

		for ((i = 0; i < 20; i++))
		{
			echo 1 > "$TRACING_PATH"/events/$dir/enable
			echo 0 > "$TRACING_PATH"/events/$dir/enable
		}
	done

	for event in `cat $TRACING_PATH/available_events`;
	do
		echo $event >> "$TRACING_PATH"/set_event
	done

	sleep 1
}

