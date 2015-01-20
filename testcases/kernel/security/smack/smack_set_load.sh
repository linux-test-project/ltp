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

export TCID=smack_set_load
export TST_TOTAL=1

. test.sh

. smack_common.sh

rule_a="TheOne                  TheOther                rwxa"
rule_b="TheOne                  TheOther                r---"

old_rule=$(grep "^TheOne" "$smackfsdir/load" 2>/dev/null | grep ' TheOther ')

echo -n "$rule_a" 2>/dev/null > "$smackfsdir/load"
new_rule=$(grep "^TheOne" "$smackfsdir/load" 2>/dev/null | grep ' TheOther ')
if [ "$new_rule" = "" ]; then
	tst_brkm TFAIL "Rule did not get set."
fi
mode=$(echo "$new_rule" | sed -e 's/.* //')
if [ "$mode" != "rwxa" ]; then
	tst_brkm TFAIL "Rule \"$new_rule\" is not set correctly."
fi

echo -n "$rule_b" 2>/dev/null > "$smackfsdir/load"
new_rule=$(grep "^TheOne" "$smackfsdir/load" 2>/dev/null | grep ' TheOther ')
if [ "$new_rule" = "" ]; then
	tst_brkm TFAIL "Rule did not get set."
fi
mode=$(echo "$new_rule" | sed -e 's/.* //')
if [ "$mode" != "r" ]; then
	tst_brkm TFAIL "Rule \"$new_rule\" is not set correctly."
fi

if [ "$old_rule" != "$new_rule" ]; then
	tst_resm TINFO "Notice: Test access rule changed from \"$old_rule\"" \
		       "to \"$new_rule\"."
fi

tst_resm TPASS "Test \"$TCID\" success."
tst_exit
