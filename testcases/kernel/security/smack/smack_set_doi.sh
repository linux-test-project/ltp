#!/bin/sh
#
# Copyright (c) 2009 Casey Schaufler under the terms of the
# GNU General Public License version 2, as published by the
# Free Software Foundation
#
# Test setting of the CIPSO doi
#
# Environment:
#	CAP_MAC_ADMIN
#

source smack_common.sh

NotTheStartValue="17"
StartValue=`cat "$smackfsdir/doi" 2>/dev/null`

echo "$NotTheStartValue" 2>/dev/null > "$smackfsdir/doi"

DirectValue=`cat "$smackfsdir/doi" 2>/dev/null`
if [ "$DirectValue" != "$NotTheStartValue" ]; then
	cat <<EOM
The CIPSO doi reported is "$DirectValue",
not the expected "$NotTheStartValue".
EOM
	exit 1
fi

echo "$StartValue" 2>/dev/null > "$smackfsdir/doi"

DirectValue=`cat "$smackfsdir/doi" 2>/dev/null`
if [ "$DirectValue" != "$StartValue" ]; then
	cat <<EOM
The CIPSO doi reported is "$DirectValue",
not the expected "$StartValue".
EOM
	exit 1
fi

exit 0
