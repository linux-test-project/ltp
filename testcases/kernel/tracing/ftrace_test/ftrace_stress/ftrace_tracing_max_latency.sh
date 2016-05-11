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

while true; do
	i=0
	while [ $i -lt $MAX_LATENCY ]; do
		echo $i > "$TRACING_PATH"/tracing_max_latency
		i=$((i + 400))
	done

	sleep 1

done
