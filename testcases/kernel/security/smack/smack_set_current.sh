#!/bin/sh
#
# Copyright (c) 2009 Casey Schaufler under the terms of the
# GNU General Public License version 2, as published by the
# Free Software Foundation
#
# Test reading of the current process Smack label
#
# Environment:
#	CAP_MAC_ADMIN
#	/smack/onlycap unset
#

export TCID=smack_set_current
export TST_TOTAL=1

. test.sh

. smack_common.sh

not_floor_label="XYZZY"
start_label=$(cat /proc/self/attr/current 2>/dev/null)

echo "$not_floor_label" 2>/dev/null > /proc/self/attr/current

label=$(cat /proc/self/attr/current 2>/dev/null)
if [ "$label" != "$not_floor_label" ]; then
	tst_brkm TFAIL "The smack label reported for the current process is" \
		       "\"$label\", not the expected \"$not_floor_label\"."
fi

echo "$start_label" 2>/dev/null > /proc/self/attr/current

label=$(cat /proc/self/attr/current 2> /dev/null)
if [ "$label" != "$start_label" ]; then
	tst_brkm TFAIL "The smack label reported for the current process is" \
		       "\"$label\", not the expected \"$start_label\"."
fi

tst_resm TPASS "Test \"$TCID\" success."
tst_exit
