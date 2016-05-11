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

MAX_LATENCY=100000

if [ ! -e "$TRACING_PATH"/tracing_max_latency ]; then
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

	for ((i = 0; i < $MAX_LATENCY; i += 400))
	{
		echo $i > "$TRACING_PATH"/tracing_max_latency
	}

	sleep 1
}

