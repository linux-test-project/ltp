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

RuleA="TheOne                  TheOther                r---"
RuleB="TheOne                  TheOther                rw--"

Where="./testdir"
What="testfile"
TestFile="$Where/$What"
CAT=/bin/cat

if [ ! -d "$Where" ]; then
	if [ -e "$Where" ]; then
		echo "Test directory \"$Where\" exists but is not a directory."
		exit 1
	fi
	mkdir -m 777 "$Where"
	if [ ! -d "$Where" ]; then
		echo "Test directory \"$Where\" can not be created."
		exit 1
	fi
fi

if [ ! -f "$TestFile" ]; then
	if [ -e "$TestFile" ]; then
		echo "Test file \"$TestFile\" exists but is not a file."
		rm -rf "$Where"
		exit 1
	fi
	./notroot /bin/sh -c "echo InitialData 2>/dev/null > $TestFile"
	if [ ! -d "$TestFile" ]; then
		echo "Test file \"$TestFile\" can not be created."
		rm -rf "$Where"
		exit 1
	fi
fi

setfattr --name=security.SMACK64 --value=TheOther "$TestFile"
SetTo=`getfattr --only-values -n security.SMACK64 -e text $TestFile`
SetTo=`echo $SetTo`

if [ "TheOther" != "$SetTo" ]; then
	echo "Test file \"$TestFile\" labeled \"$SetTo\" incorrectly."
	rm -rf "$Where"
	exit 1
fi

OldRule=`grep "^TheOne" "$smackfsdir/load" 2>/dev/null | grep ' TheOther '`

echo -n "$RuleA" > "$smackfsdir/load"
NewRule=`grep "^TheOne" "$smackfsdir/load" 2>/dev/null | grep ' TheOther '`
if [ "$NewRule" = "" ]; then
	echo "Rule did not get set."
	rm -rf "$Where"
	exit 1
fi
Mode=`echo $NewRule | sed -e 's/.* //'`
if [ "$Mode" != "r" ]; then
	echo "Rule \"$NewRule\" is not set correctly."
	rm -rf "$Where"
	exit 1
fi

OldProc=`cat /proc/self/attr/current 2>/dev/null`

echo TheOne 2>/dev/null > /proc/self/attr/current
GotRead=`./notroot $CAT "$TestFile"`

if [ "$GotRead" != "InitialData" ]; then
	echo "Read failed for \"$TestFile\" labeled \"TheOther\"."
	rm -rf "$Where"
	exit 1
fi

echo NotTheOne 2>/dev/null > /proc/self/attr/current
GotRead=`./notroot $CAT "$TestFile"`

if [ "$GotRead" = "InitialData" ]; then
	echo "Read should have failed for \"$TestFile\" labeled \"TheOther\"."
	rm -rf "$Where"
	exit 1
fi

echo -n "$RuleB" 2>/dev/null > "$smackfsdir/load"
NewRule=`grep "^TheOne" $smackfsdir/load 2>/dev/null | grep ' TheOther '`
if [ "$NewRule" = "" ]; then
	echo "Rule did not get set."
	rm -rf "$Where"
	exit 1
fi
Mode=`echo $NewRule | sed -e 's/.* //'`
if [ "$Mode" != "rw" ]; then
	echo "Rule \"$NewRule\" is not set correctly."
	rm -rf "$Where"
	exit 1
fi

if [ "$OldRule" != "$NewRule" ]; then
	cat <<EOM
Notice: Test access rule changed from "$OldRule" to "$NewRule".
EOM
fi

rm -rf "$Where"
