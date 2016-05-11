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

# Use up to 10% of free memory
free_mem=`cat /proc/meminfo | grep '^MemFree' | awk '{ print $2 }'`
cpus=`tst_ncpus`

step=$(( $free_mem / 10 / $LOOP / $cpus ))

if [ $step -eq 0 ]; then
	step=1
	LOOP=50
fi

while true; do
	new_size=1
	i=0
	while [ $i -lt $LOOP ]; do
		echo $new_size > "$TRACING_PATH"/buffer_size_kb
		new_size=$(( $new_size + $step ))
		i=$((i + 1))
	done

	i=0
	while [ $i -lt $LOOP ]; do
		new_size=$(( $new_size - $step ))
		echo $new_size > "$TRACING_PATH"/buffer_size_kb
		i=$((i + 1))
	done
	sleep 1
done
