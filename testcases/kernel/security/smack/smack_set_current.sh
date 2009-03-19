#!/bin/sh
#
# Copyright (c) 2009 Casey Schaufler under the terms of the
# GNU General Public License version 2, as published by the
# Free Software Foundation
#
# Test reading of the current process Smack label
#
# Environment:
#	CAP_MAC_ADMIN
#	/smack/onlycap unset
#
NotTheFloorLabel="XYZZY"
StartLabel=`cat /proc/self/attr/current`

onlycap=`cat /smack/onlycap`
if [ "$onlycap" != "" ]; then
	echo The smack label reported for /smack/onlycap is \"$label\",
	echo not the expected \"\".
	exit 1
fi

echo $NotTheFloorLabel > /proc/self/attr/current

label=`cat /proc/self/attr/current`
if [ "$label" != "$NotTheFloorLabel" ]; then
	echo The smack label reported for the current process is \"$label\",
	echo not the expected \"$NotTheFloorLabel\".
	exit 1
fi

echo "$StartLabel" > /proc/self/attr/current

label=`cat /proc/self/attr/current`
if [ "$label" != "$StartLabel" ]; then
	echo The smack label reported for the current process is \"$label\",
	echo not the expected \"$StartLabel\".
	exit 1
fi

exit 0
