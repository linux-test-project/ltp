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

ftrace_sleep()
{
	# usleep is not a standard command?
	usleep 200000 2> /dev/null
	if [ $? -ne 0 ]; then
		sleep 1
	fi
}

kill_this_pid()
{
	/bin/kill -SIGKILL $this_pid
	wait $this_pid
	exit 0
}

trap kill_this_pid SIGUSR1

LOOP=20

for ((; ;))
{
	for ((i = 0; i < $LOOP; i++))
	{
		cat "$TRACING_PATH"/trace_pipe > /dev/null &

		this_pid=$!
		ftrace_sleep
		/bin/kill -SIGINT $this_pid
		wait $this_pid
		this_pid=0
		ftrace_sleep
	}

	sleep 2
}

