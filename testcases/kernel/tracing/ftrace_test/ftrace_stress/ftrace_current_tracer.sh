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

while true; do
	i=0
	while [ $i -lt $LOOP ]; do
		for tracer in `cat "$TRACING_PATH"/available_tracers`
		do
			if [ "$tracer" = mmiotrace ]; then
				continue
			fi

			echo $tracer > "$TRACING_PATH"/current_tracer 2> /dev/null
		done
		i=$((i + 1))
	done
	sleep 1
done
