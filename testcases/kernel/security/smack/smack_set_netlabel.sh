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
RuleA="191.191.191.191 TheOne"
RuleA1="191.191.191.191/32 TheOne"
RuleB="191.190.190.0/24 TheOne"

onlycap=`cat /smack/onlycap`
if [ "$onlycap" != "" ]; then
	echo The smack label reported for /smack/onlycap is \"$label\",
	echo not the expected \"\".
	exit 1
fi

Old32=`grep "^191.191.191.191/32" /smack/netlabel`
Old24=`grep "^191.190.190.0/24" /smack/netlabel`

echo -n "$RuleA" > /smack/netlabel
New32=`grep "$RuleA1" /smack/netlabel`
if [ "$New32" != "$RuleA1" ]; then
	echo Rule \"$RuleA\" did not get set.
	exit 1
fi

echo -n "$RuleB" > /smack/netlabel
New24=`grep "$RuleB" /smack/netlabel`
if [ "$New24" != "$RuleB" ]; then
	echo Rule \"$RuleB\" did not get set.
	exit 1
fi

if [ "$Old24" != "$New24" ]; then
	echo Notice: Test access rule changed from
	echo \"$Old24\" to \"$New24\".
fi
if [ "$Old32" != "$New32" ]; then
	echo Notice: Test access rule changed from
	echo \"$Old32\" to \"$New32\".
fi

exit 0
