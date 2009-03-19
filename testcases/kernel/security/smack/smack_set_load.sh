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
RuleA="TheOne                  TheOther                rwxa"
RuleB="TheOne                  TheOther                r---"

onlycap=`cat /smack/onlycap`
if [ "$onlycap" != "" ]; then
	echo The smack label reported for /smack/onlycap is \"$label\",
	echo not the expected \"\".
	exit 1
fi

OldRule=`grep "^TheOne" /smack/load | grep ' TheOther '`

echo -n "$RuleA" > /smack/load
NewRule=`grep "^TheOne" /smack/load | grep ' TheOther '`
if [ "$NewRule" == "" ]; then
	echo Rule did not get set.
	exit 1
fi
Mode=`echo $NewRule | sed -e 's/.* //'`
if [ "$Mode" != "rwxa" ]; then
	echo Rule \"$NewRule\" is not set correctly.
	exit 1
fi

echo -n "$RuleB" > /smack/load
NewRule=`grep "^TheOne" /smack/load | grep ' TheOther '`
if [ "$NewRule" == "" ]; then
	echo Rule did not get set.
	exit 1
fi
Mode=`echo $NewRule | sed -e 's/.* //'`
if [ "$Mode" != "r" ]; then
	echo Rule \"$NewRule\" is not set correctly.
	exit 1
fi

if [ "$OldRule" != "$NewRule" ]; then
	echo Notice: Test access rule changed from
	echo \"$OldRule\" to \"$NewRule\".
fi

exit 0
