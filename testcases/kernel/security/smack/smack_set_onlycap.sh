#!/bin/sh
#
# Copyright (c) 2009 Casey Schaufler under the terms of the
# GNU General Public License version 2, as published by the
# Free Software Foundation
#
# Test setting of the privileged Smack label
#
# Environment:
#	CAP_MAC_ADMIN
#
MyLabel=`cat /proc/self/attr/current`
StartLabel=`cat /smack/onlycap`

if [ "$StartLabel" != "" ]; then
	echo The smack label reported for /smack/onlycap is \"$StartLabel\",
	echo not the expected \"\".
	exit 1
fi

echo $MyLabel > /smack/onlycap

label=`cat /smack/onlycap`
if [ "$label" != "$MyLabel" ]; then
	echo The smack label reported for /smack/onlycap is \"$label\",
	echo not the expected \"$MyLabel\".
	exit 1
fi

echo "$StartLabel" > /smack/onlycap

label=`cat /smack/onlycap`
if [ "$label" != "$StartLabel" ]; then
	echo The smack label reported for the current process is \"$label\",
	echo not the expected \"$StartLabel\".
	exit 1
fi

exit 0
