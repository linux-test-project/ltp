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
# "%-23s %-23s %4s"
#
#               1         2         3         4         5         6
#      123456789012345678901234567890123456789012345678901234567890123456789

source smack_common.sh

RuleA="TheOne                  TheOther                rwxa"
RuleB="TheOne                  TheOther                r---"

OldRule=`grep "^TheOne" "$smackfsdir/load" 2>/dev/null | grep ' TheOther '`

echo -n "$RuleA" 2>/dev/null > "$smackfsdir/load"
NewRule=`grep "^TheOne" "$smackfsdir/load" 2>/dev/null | grep ' TheOther '`
if [ "$NewRule" = "" ]; then
	echo "Rule did not get set."
	exit 1
fi
Mode=`echo "$NewRule" | sed -e 's/.* //'`
if [ "$Mode" != "rwxa" ]; then
	echo "Rule \"$NewRule\" is not set correctly."
	exit 1
fi

echo -n "$RuleB" 2>/dev/null > "$smackfsdir/load"
NewRule=`grep "^TheOne" "$smackfsdir/load" 2>/dev/null | grep ' TheOther '`
if [ "$NewRule" = "" ]; then
	echo "Rule did not get set."
	exit 1
fi
Mode=`echo "$NewRule" | sed -e 's/.* //'`
if [ "$Mode" != "r" ]; then
	echo "Rule \"$NewRule\" is not set correctly."
	exit 1
fi

if [ "$OldRule" != "$NewRule" ]; then
	cat <<EOM
Notice: Test access rule changed from
"$OldRule" to "$NewRule".
EOM
fi
