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

source smack_common.sh

NotTheFloorLabel="XYZZY"
StartLabel=`cat "$smackfsdir/ambient" 2>/dev/null`

echo "$NotTheFloorLabel" 2>/dev/null > "$smackfsdir/ambient"

label=`cat "$smackfsdir/ambient" 2>/dev/null`
if [ "$label" != "$NotTheFloorLabel" ]; then
	cat <<EOM
The smack label reported for the current process is "$label", not the expected 
"$NotTheFloorLabel".
EOM
	exit 1
fi

echo "$StartLabel" 2>/dev/null > "$smackfsdir/ambient"

label=`cat "$smackfsdir/ambient" 2>/dev/null`
if [ "$label" != "$StartLabel" ]; then
	cat <<EOM
The smack label reported for the current process is "$label",  not the expected "$StartLabel".
EOM
	exit 1
fi
