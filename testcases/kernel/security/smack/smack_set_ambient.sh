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
#
NotTheFloorLabel="XYZZY"
StartLabel=`cat /smack/ambient`

onlycap=`cat /smack/onlycap`
if [ "$onlycap" != "" ]; then
	echo The smack label reported for /smack/onlycap is \"$label\",
	echo not the expected \"\".
	exit 1
fi

echo $NotTheFloorLabel > /smack/ambient

label=`cat /smack/ambient`
if [ "$label" != "$NotTheFloorLabel" ]; then
	echo The smack label reported for the current process is \"$label\",
	echo not the expected \"$NotTheFloorLabel\".
	exit 1
fi

echo "$StartLabel" > /smack/ambient

label=`cat /smack/ambient`
if [ "$label" != "$StartLabel" ]; then
	echo The smack label reported for the current process is \"$label\",
	echo not the expected \"$StartLabel\".
	exit 1
fi

exit 0
