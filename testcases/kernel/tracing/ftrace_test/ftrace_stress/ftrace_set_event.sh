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

while true; do
	i=0
	while [ $i -lt 100 ]; do
		echo 1 > "$TRACING_PATH"/events/enable
		echo 0 > "$TRACING_PATH"/events/enable
		i=$((i + 1))
	done

	for dir in `ls $TRACING_PATH/events/`; do
		if [ ! -d $dir -o "$dir" = ftrace ]; then
			continue;
		fi

		i=0
		while [ $i -lt 20 ]; do
			echo 1 > "$TRACING_PATH"/events/$dir/enable
			echo 0 > "$TRACING_PATH"/events/$dir/enable
			i=$((i + 1))
		done
	done

	for event in `cat $TRACING_PATH/available_events`; do
		# ftrace event sys is special, skip it
		if echo "$event" | grep "ftrace:*"; then
			continue
		fi
		echo $event >> "$TRACING_PATH"/set_event
	done

	sleep 1
done
