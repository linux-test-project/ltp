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
tst_kvercmp 2 6 32
if [ $? -eq 0 ]; then
        old_kernel=1
else
        old_kernel=0
fi

for ((; ;))
{
	if [ $old_kernel -eq 1 ];
	then
		for ((i = 0; i < $LOOP; i++))
		{
			echo 1 > "$TRACING_PATH"/options/global-clock
			echo 0 > "$TRACING_PATH"/options/global-clock
		}
	else
		for ((i = 0; i < $LOOP; i++))
		{
			echo local > "$TRACING_PATH"/trace_clock
			echo global > "$TRACING_PATH"/trace_clock
		}
	fi

	sleep 1
}

