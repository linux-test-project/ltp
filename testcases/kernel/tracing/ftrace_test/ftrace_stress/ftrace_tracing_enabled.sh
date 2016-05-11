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

while true; do
	count=$(( $count + 1 ))
	i=0
	while [ $i -lt $MAX_LOOP ]; do
		echo 0 > "$TRACING_PATH"/tracing_enabled
		echo 1 > "$TRACING_PATH"/tracing_enabled
		i=$((i + 1))
	done

	enable=$(( $count % 3 ))

	if [ $enable -eq 0 ]; then
		echo 0 > "$TRACING_PATH"/tracing_enabled
	else
		echo 1 > "$TRACING_PATH"/tracing_enabled
	fi

	sleep 1
done

