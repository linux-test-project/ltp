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

source smack_common.sh

RuleA="TheOne                  2   0   "
RuleB="TheOne                  3   1   55  "
RuleC="TheOne                  4   2   17  33  "

OldRule=`grep "^TheOne" "$smackfsdir/cipso" 2>/dev/null`

echo -n "$RuleA" 2>/dev/null > "$smackfsdir/cipso"
NewRule=`grep "^TheOne" "$smackfsdir/cipso" 2>/dev/null`
if [ "$NewRule" = "" ]; then
	echo "Rule did not get set."
	exit 1
fi
Right=`echo "$NewRule" | grep ' 2'`
if [ "$Right" = "" ]; then
	echo "Rule \"$NewRule\" is not set correctly."
	exit 1
fi

echo -n "$RuleB" 2>/dev/null > "$smackfsdir/cipso"
NewRule=`grep "^TheOne" "$smackfsdir/cipso" 2>/dev/null`
if [ "$NewRule" = "" ]; then
	echo "Rule did not get set."
	exit 1
fi
Right=`echo $NewRule | grep '/55'`
if [ "$Right" = "" ]; then
	echo "Rule \"$NewRule\" is not set correctly."
	exit 1
fi

echo -n "$RuleC" 2>/dev/null > "$smackfsdir/cipso"
NewRule=`grep "^TheOne" "$smackfsdir/cipso" 2>/dev/null`
if [ "$NewRule" = "" ]; then
	echo "Rule did not get set."
	exit 1
fi
Right=`echo "$NewRule" | grep '/17,33'`
if [ "$Right" = "" ]; then
	echo "Rule \"$NewRule\" is not set correctly."
	exit 1
fi

if [ "$OldRule" != "$NewRule" ]; then
	cat <<EOM
Notice: Test access rule changed from "$OldRule" to "$NewRule".
EOM
fi
