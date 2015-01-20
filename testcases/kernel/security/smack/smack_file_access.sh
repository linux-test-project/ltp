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

export TCID=smack_file_access
export TST_TOTAL=1

. test.sh

. smack_common.sh

cleanup()
{
	tst_rmdir
}

rule_a="TheOne                  TheOther                r---"
rule_b="TheOne                  TheOther                rw--"

CAT=/bin/cat
testfile="testfile"

tst_tmpdir
TST_CLEANUP=cleanup

smack_notroot /bin/sh -c "echo InitialData 2>/tmp/smack_fail.log > $testfile"
if [ ! -f "$testfile" ]; then
	tst_brkm TFAIL "Test file \"$testfile\" can not be created."
fi

setfattr --name=security.SMACK64 --value=TheOther "$testfile"
setto=$(getfattr --only-values -n security.SMACK64 -e text $testfile)

if [ "TheOther" != "$setto" ]; then
	tst_brkm TFAIL "Test file \"$testfile\" labeled \"$setto\" incorrectly."
fi

old_rule=$(grep "^TheOne" "$smackfsdir/load" 2>/dev/null | grep ' TheOther ')

echo -n "$rule_a" > "$smackfsdir/load"
new_rule=$(grep "^TheOne" "$smackfsdir/load" 2>/dev/null | grep ' TheOther ')
if [ "$new_rule" = "" ]; then
	tst_brkm TFAIL "Rule did not get set."
fi
mode=$(echo $new_rule | sed -e 's/.* //')
if [ "$mode" != "r" ]; then
	tst_brkm TFAIL "Rule \"$new_rule\" is not set correctly."
fi

echo TheOne 2>/dev/null > /proc/self/attr/current
got_read=$(smack_notroot $CAT "$testfile")

if [ "$got_read" != "InitialData" ]; then
	tst_brkm TFAIL "Read failed for \"$testfile\" labeled \"TheOther\"."
fi

echo NotTheOne 2>/dev/null > /proc/self/attr/current
got_read=$(smack_notroot $CAT "$testfile" 2> /dev/null)

if [ "$got_read" = "InitialData" ]; then
	tst_brkm TFAIL "Read should have failed for \"$testfile\" labeled" \
		       "\"TheOther\"."
fi

echo -n "$rule_b" 2>/dev/null > "$smackfsdir/load"
new_rule=$(grep "^TheOne" $smackfsdir/load 2>/dev/null | grep ' TheOther ')
if [ "$new_rule" = "" ]; then
	tst_brkm TFAIL "Rule did not get set."
fi
mode=$(echo $new_rule | sed -e 's/.* //')
if [ "$mode" != "rw" ]; then
	tst_brkm TFAIL "Rule \"$new_rule\" is not set correctly."
fi

if [ "$old_rule" != "$new_rule" ]; then
	tst_resm TINFO "Notice: Test access rule changed from \"$old_rule\"" \
		       "to \"$new_rule\"."
fi

tst_resm TPASS "Test \"$TCID\" success."
tst_exit
