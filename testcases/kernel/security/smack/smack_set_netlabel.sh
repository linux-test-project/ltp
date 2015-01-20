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

export TCID=smack_set_netlabel
export TST_TOTAL=1

. test.sh

. smack_common.sh

rule_a="191.191.191.191 TheOne"
rule_a1="191.191.191.191/32 TheOne"
rule_b="191.190.190.0/24 TheOne"

old32=$(grep "^191.191.191.191/32" "$smackfsdir/netlabel" 2>/dev/null)
old24=$(grep "^191.190.190.0/24" "$smackfsdir/netlabel" 2>/dev/null)

echo -n "$rule_a" 2>/dev/null > "$smackfsdir/netlabel"
new32=$(grep "$rule_a1" $smackfsdir/netlabel 2>/dev/null)
if [ "$new32" != "$rule_a1" ]; then
	tst_brkm TFAIL "Rule \"$rule_a\" did not get set."
fi

echo -n "$rule_b" 2>/dev/null > "$smackfsdir/netlabel"
new24=$(grep "$rule_b" "$smackfsdir/netlabel" 2>/dev/null)
if [ "$new24" != "$rule_b" ]; then
	tst_brkm TFAIL "Rule \"$rule_b\" did not get set."
fi

if [ "$old24" != "$new24" ]; then
	tst_resm TINFO "Notice: Test access rule changed from \"$old24\" to" \
		       "\"$new24\"."
fi

if [ "$old32" != "$new32" ]; then
	tst_resm TINFO "Notice: Test access rule changed from \"$old32\" to \
\"$new32\"."
fi

tst_resm TPASS "Test \"$TCID\" success."
tst_exit
