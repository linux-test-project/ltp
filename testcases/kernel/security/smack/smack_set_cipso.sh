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
#               1         2    llllCCCCccccCCCCcccc     5         6
#      123456789012345678901234567890123456789012345678901234567890123456789
RuleA="TheOne                  2   0   "
RuleB="TheOne                  3   1   55  "
RuleC="TheOne                  4   2   17  33  "

onlycap=`cat /smack/onlycap`
if [ "$onlycap" != "" ]; then
	echo The smack label reported for /smack/onlycap is \"$label\",
	echo not the expected \"\".
	exit 1
fi

OldRule=`grep "^TheOne" /smack/cipso`

echo -n "$RuleA" > /smack/cipso
NewRule=`grep "^TheOne" /smack/cipso`
if [ "$NewRule" == "" ]; then
	echo Rule did not get set.
	exit 1
fi
Right=`echo $NewRule | grep ' 2'`
if [ "$Right" == "" ]; then
	echo Rule \"$NewRule\" is not set correctly.
	exit 1
fi

echo -n "$RuleB" > /smack/cipso
NewRule=`grep "^TheOne" /smack/cipso`
if [ "$NewRule" == "" ]; then
	echo Rule did not get set.
	exit 1
fi
Right=`echo $NewRule | grep '/55'`
if [ "$Right" == "" ]; then
	echo Rule \"$NewRule\" is not set correctly.
	exit 1
fi

echo -n "$RuleC" > /smack/cipso
NewRule=`grep "^TheOne" /smack/cipso`
if [ "$NewRule" == "" ]; then
	echo Rule did not get set.
	exit 1
fi
Right=`echo $NewRule | grep '/17,33'`
if [ "$Right" == "" ]; then
	echo Rule \"$NewRule\" is not set correctly.
	exit 1
fi


if [ "$OldRule" != "$NewRule" ]; then
	echo Notice: Test access rule changed from \"$OldRule\" to \"$NewRule\".
fi

exit 0
