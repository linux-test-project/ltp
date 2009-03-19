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
RuleA="TheOne                  TheOther                r---"
RuleB="TheOne                  TheOther                rw--"

Where="./testdir"
What="testfile"
TestFile="$Where"/"$What"
CAT=/bin/cat

onlycap=`cat /smack/onlycap`
if [ "$onlycap" != "" ]; then
	echo The smack label reported for /smack/onlycap is \"$label\",
	echo not the expected \"\".
	exit 1
fi

if [ ! -d "$Where" ]; then
	if [ -e "$Where" ]; then
		echo Test directory \"$Where\" exists but is not a directory.
		exit 1
	fi
	mkdir "$Where"
	if [ ! -d "$Where" ]; then
		echo Test directory \"$Where\" can not be created.
		exit 1
	fi
	chmod 777 "$Where"
fi

if [ ! -f "$TestFile" ]; then
	if [ -e "$TestFile" ]; then
		echo Test file \"$TestFile\" exists but is not a file.
		rm -rf "$Where"
		exit 1
	fi
	./notroot /bin/sh -c "echo InitialData > $TestFile"
	if [ ! -d "$TestFile" ]; then
		echo Test file \"$TestFile\" can not be created.
		rm -rf "$Where"
		exit 1
	fi
fi

setfattr --name=security.SMACK64 --value=TheOther "$TestFile"
SetTo=`getfattr --only-values -n security.SMACK64 -e text $TestFile`
SetTo=`echo $SetTo`

if [ "TheOther" != "$SetTo" ]; then
	echo Test file \"$TestFile\" labeled \"$SetTo\" incorrectly.
	rm -rf "$Where"
	exit 1
fi

OldRule=`grep "^TheOne" /smack/load | grep ' TheOther '`

echo -n "$RuleA" > /smack/load
NewRule=`grep "^TheOne" /smack/load | grep ' TheOther '`
if [ "$NewRule" == "" ]; then
	echo Rule did not get set.
	rm -rf "$Where"
	exit 1
fi
Mode=`echo $NewRule | sed -e 's/.* //'`
if [ "$Mode" != "r" ]; then
	echo Rule \"$NewRule\" is not set correctly.
	rm -rf "$Where"
	exit 1
fi

OldProc=`cat /proc/self/attr/current`

echo TheOne > /proc/self/attr/current
GotRead=`./notroot $CAT "$TestFile"`

if [ "$GotRead" != "InitialData" ]; then
	echo Read failed for \"$TestFile\" labeled \"TheOther\".
	rm -rf "$Where"
	exit 1
fi

echo NotTheOne > /proc/self/attr/current
GotRead=`./notroot $CAT "$TestFile"`

if [ "$GotRead" == "InitialData" ]; then
	echo Read should have failed for \"$TestFile\" labeled \"TheOther\".
	rm -rf "$Where"
	exit 1
fi

echo -n "$RuleB" > /smack/load
NewRule=`grep "^TheOne" /smack/load | grep ' TheOther '`
if [ "$NewRule" == "" ]; then
	echo Rule did not get set.
	rm -rf "$Where"
	exit 1
fi
Mode=`echo $NewRule | sed -e 's/.* //'`
if [ "$Mode" != "rw" ]; then
	echo Rule \"$NewRule\" is not set correctly.
	rm -rf "$Where"
	exit 1
fi

if [ "$OldRule" != "$NewRule" ]; then
	echo Notice: Test access rule changed from
	echo \"$OldRule\" to \"$NewRule\".
fi

rm -rf "$Where"
exit 0
