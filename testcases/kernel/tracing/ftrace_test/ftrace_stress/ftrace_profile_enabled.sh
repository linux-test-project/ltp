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

if [ ! -e "$TRACING_PATH"/function_profile_enabled ]; then
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

	count=$(( $count + 1 ))

	for ((i = 0; i < $MAX_LOOP; i++))
	{
		echo 0 > "$TRACING_PATH"/function_profile_enabled 2> /dev/null
		echo 1 > "$TRACING_PATH"/function_profile_enabled 2> /dev/null
	}

	enable=$(( $count % 3 ))

	if [ $enable -eq 0 ]; then
		echo 1 > "$TRACING_PATH"/function_profile_enabled 2> /dev/null
	else
		echo 0 > "$TRACING_PATH"/function_profile_enabled 2> /dev/null
	fi

	sleep 1
}

