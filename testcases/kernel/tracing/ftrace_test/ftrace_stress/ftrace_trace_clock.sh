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

# In kernel which is older than 2.6.32, we set global clock
# via trace_options.
if tst_kvcmp -lt "2.6.32"; then
        old_kernel=1
else
        old_kernel=0
fi

while true; do
	i=0
	if [ $old_kernel -eq 1 ]; then
		while [ $i -lt $LOOP ]; do
			echo 1 > "$TRACING_PATH"/options/global-clock
			echo 0 > "$TRACING_PATH"/options/global-clock
			i=$((i + 1))
		done
	else
		while [ $i -lt $LOOP ]; do
			echo local > "$TRACING_PATH"/trace_clock
			echo global > "$TRACING_PATH"/trace_clock
			i=$((i + 1))
		done

	fi

	sleep 1
done

