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

while true; do
	i=0
	while [ $i -lt $LOOP ]; do
		cat "$TRACING_PATH"/stack_trace > /dev/null
		i=$((i + 1))
	done

	sleep 1

	i=0
	while [ $i -lt $LOOP ]; do
		echo 0 > /proc/sys/kernel/stack_tracer_enabled
		echo 1 > /proc/sys/kernel/stack_tracer_enabled
		i=$((i + 1))
	done

	sleep 1
done
