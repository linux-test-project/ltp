#!/bin/sh
#
# Copyright (c) 2009 Casey Schaufler under the terms of the
# GNU General Public License version 2, as published by the
# Free Software Foundation
#
# Test setting access rules
#
# Environment:
#	CAP_MAC_ADMIN
#

source smack_common.sh

RuleA="191.191.191.191 TheOne"
RuleA1="191.191.191.191/32 TheOne"
RuleB="191.190.190.0/24 TheOne"

Old32=`grep "^191.191.191.191/32" "$smackfsdir/netlabel" 2>/dev/null`
Old24=`grep "^191.190.190.0/24" "$smackfsdir/netlabel" 2>/dev/null`

echo -n "$RuleA" 2>/dev/null > "$smackfsdir/netlabel"
New32=`grep "$RuleA1" $smackfsdir/netlabel 2>/dev/null`
if [ "$New32" != "$RuleA1" ]; then
	echo "Rule \"$RuleA\" did not get set."
	exit 1
fi

echo -n "$RuleB" 2>/dev/null > "$smackfsdir/netlabel"
New24=`grep "$RuleB" "$smackfsdir/netlabel" 2>/dev/null`
if [ "$New24" != "$RuleB" ]; then
	echo "Rule \"$RuleB\" did not get set."
	exit 1
fi

if [ "$Old24" != "$New24" ]; then
	cat <<EOM
Notice: Test access rule changed from
"$Old24" to "$New24".
EOM
fi

if [ "$Old32" != "$New32" ]; then
	cat <<EOM
Notice: Test access rule changed from
"$Old32" to "$New32".
EOM
fi
