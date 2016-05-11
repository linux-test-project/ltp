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

MAX_STACK_SIZE=8192

while true; do
	i=0
	while [ $i -lt $MAX_STACK_SIZE ]; do
		echo $i > "$TRACING_PATH"/stack_max_size
		cat "$TRACING_PATH"/stack_max_size > /dev/null
		i=$((i + 1))
	done
	sleep 1
done

