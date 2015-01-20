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

export TCID=smack_set_cipso
export TST_TOTAL=1

. test.sh

. smack_common.sh

rule_a="TheOne                  2   0   "
rule_b="TheOne                  3   1   55  "
rule_c="TheOne                  4   2   17  33  "

old_rule=$(grep "^TheOne" "$smackfsdir/cipso" 2>/dev/null)

echo -n "$rule_a" 2>/dev/null > "$smackfsdir/cipso"
new_rule=$(grep "^TheOne" "$smackfsdir/cipso" 2>/dev/null)
if [ "$new_rule" = "" ]; then
	tst_brkm TFAIL "Rule did not get set."
fi
right=$(echo "$new_rule" | grep ' 2')
if [ "$right" = "" ]; then
	tst_brkm TFAIL "Rule \"$new_rule\" is not set correctly."
fi

echo -n "$rule_b" 2>/dev/null > "$smackfsdir/cipso"
new_rule=$(grep "^TheOne" "$smackfsdir/cipso" 2>/dev/null)
if [ "$new_rule" = "" ]; then
	tst_brkm TFAIL "Rule did not get set."
fi
right=$(echo $new_rule | grep '/55')
if [ "$right" = "" ]; then
	tst_brkm TFAIL "Rule \"$new_rule\" is not set correctly."
fi

echo -n "$rule_c" 2>/dev/null > "$smackfsdir/cipso"
new_rule=$(grep "^TheOne" "$smackfsdir/cipso" 2>/dev/null)
if [ "$new_rule" = "" ]; then
	tst_brkm TFAIL "Rule did not get set."
fi
right=$(echo "$new_rule" | grep '/17,33')
if [ "$right" = "" ]; then
	tst_brkm TFAIL "Rule \"$new_rule\" is not set correctly."
fi

if [ "$old_rule" != "$new_rule" ]; then
	tst_resm TINFO "Notice: Test access rule changed from \"$old_rule\"" \
		       "to \"$new_rule\"."
fi

tst_resm TPASS "Test \"$TCID\" success."
tst_exit
