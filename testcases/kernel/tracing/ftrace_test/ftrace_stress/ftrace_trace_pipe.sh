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

kill_this_pid()
{
	kill -KILL $this_pid
	wait $this_pid
	exit 0
}

trap kill_this_pid SIGUSR1

LOOP=20

while true; do
	i=0
	while [ $i -lt $LOOP ]; do
		cat "$TRACING_PATH"/trace_pipe > /dev/null &
		this_pid=$!

		tst_sleep 200000us

		kill -INT $this_pid
		wait $this_pid

		this_pid=0

		tst_sleep 200000us

		i=$((i + 1))
	done
	sleep 2
done
