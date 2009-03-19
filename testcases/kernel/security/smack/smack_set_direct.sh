#!/bin/sh
#
# Copyright (c) 2009 Casey Schaufler under the terms of the
# GNU General Public License version 2, as published by the
# Free Software Foundation
#
# Test setting of the CIPSO direct level
#
# Environment:
#	CAP_MAC_ADMIN
#
NotTheStartValue="17"
StartValue=`cat /smack/direct`

onlycap=`cat /smack/onlycap`
if [ "$onlycap" != "" ]; then
	echo The smack label reported for /smack/onlycap is \"$onlycap\",
	echo not the expected \"\".
	exit 1
fi

echo $NotTheStartValue > /smack/direct

DirectValue=`cat /smack/direct`
if [ "$DirectValue" != "$NotTheStartValue" ]; then
	echo The CIPSO direct level reported is \"$DirectValue\",
	echo not the expected \"$NotTheStartValue\".
	exit 1
fi

echo "$StartValue" > /smack/direct

DirectValue=`cat /smack/direct`
if [ "$DirectValue" != "$StartValue" ]; then
	echo The CIPSO direct level reported is \"$DirectValue\",
	echo not the expected \"$StartValue\".
	exit 1
fi

exit 0
