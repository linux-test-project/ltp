#!/bin/sh
#
# Copyright (c) 2009 Casey Schaufler under the terms of the
# GNU General Public License version 2, as published by the
# Free Software Foundation
#
# Test setting of the privileged Smack label
#
# Environment:
#	CAP_MAC_ADMIN
#

export TCID=smack_set_onlycap
export TST_TOTAL=1

. test.sh

. smack_common.sh

my_label=$(cat /proc/self/attr/current 2>/dev/null)
start_label=$(cat "$smackfsdir/onlycap" 2>/dev/null)

echo "$my_label" 2>/dev/null > "$smackfsdir/onlycap"

label=$(cat "$smackfsdir/onlycap" 2>/dev/null)
if [ "$label" != "$my_label" ]; then
	tst_brkm TFAIL "The smack label reported for \"$smackfsdir/onlycap\" "
		       "is \"$label\", not the expected \"$my_label\"."
fi

echo "$start_label" 2>/dev/null > "$smackfsdir/onlycap"

label=$(cat "$smackfsdir/onlycap" 2>/dev/null)
if [ "$label" != "$start_label" ]; then
	tst_brkm TFAIL "The smack label reported for the current process is "
		       "\"$label\", not the expected \"$start_label\"."
fi

tst_resm TPASS "Test \"$TCID\" success."
tst_exit
