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

source smack_common.sh

MyLabel=`cat /proc/self/attr/current 2>/dev/null`
StartLabel=`cat "$smackfsdir/onlycap" 2>/dev/null`

echo "$MyLabel" 2>/dev/null > "$smackfsdir/onlycap"

label=`cat "$smackfsdir/onlycap" 2>/dev/null`
if [ "$label" != "$MyLabel" ]; then
	cat <<EOM
The smack label reported for $smackfsdir/onlycap is "$label",
not the expected "$MyLabel".
EOM
	exit 1
fi

echo "$StartLabel" 2>/dev/null > "$smackfsdir/onlycap"

label=`cat "$smackfsdir/onlycap" 2>/dev/null`
if [ "$label" != "$StartLabel" ]; then
	cat <<EOM
The smack label reported for the current process is "$label",
not the expected "$StartLabel".
EOM
	exit 1
fi
