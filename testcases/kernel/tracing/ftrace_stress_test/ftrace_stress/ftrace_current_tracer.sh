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

LOOP=200

for ((; ;))
{
	for ((i = 0; i < $LOOP; i++))
	{
		for tracer in `cat "$TRACING_PATH"/available_tracers`
		do
			if [ "$tracer" = mmiotrace ]; then
				continue
			fi

			echo $tracer > "$TRACING_PATH"/current_tracer 2> /dev/null
		done
	}

	sleep 1
}

