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

should_skip=0
nr_cpus=`tst_ncpus`

if [ ! -e "$TRACING_PATH"/function_profile_enabled ]; then
        should_skip=1
fi

# For kernels older than 2.6.36, this testcase can result in
# divide-by-zero kernel bug
if tst_kvcmp -lt "2.6.36"; then
	should_skip=1
fi

while true; do
	if [ $should_skip -eq 1 ]; then
		sleep 2
		continue
	fi
	cpu=$(tst_random 0 $((nr_cpus - 1)))
	i=0;
	while [ $i -lt $LOOP ]; do
		cat "$TRACING_PATH"/trace_stat/function${cpu} > /dev/null 2>&1
		i=$((i + 1))
	done

	sleep 1
done
