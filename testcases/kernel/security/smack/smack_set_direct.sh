#!/bin/sh
#
# Copyright (c) 2009 Casey Schaufler under the terms of the
# GNU General Public License version 2, as published by the
# Free Software Foundation
#
# Test setting of the CIPSO direct level
#
# Environment:
#	CAP_MAC_ADMIN
#

export TCID=smack_set_direct
export TST_TOTAL=1

. test.sh

. smack_common.sh

not_start_value="17"
start_value=$(cat "$smackfsdir/direct" 2>/dev/null)

echo "$not_start_value" 2>/dev/null > "$smackfsdir/direct"

direct_value=$(cat "$smackfsdir/direct" 2>/dev/null)
if [ "$direct_value" != "$not_start_value" ]; then
	tst_brkm TFAIL "The CIPSO direct level reported is \"$direct_value\"," \
		       "not the expected \"$not_start_value\"."
fi

echo "$start_value" 2>/dev/null> "$smackfsdir/direct"

direct_value=$(cat "$smackfsdir/direct" 2>/dev/null)
if [ "$direct_value" != "$start_value" ]; then
	tst_brkm TFAIL "The CIPSO direct level reported is \"$direct_value\"," \
		       "not the expected \"$start_value\"."
fi

tst_resm TPASS "Test \"$TCID\" success."
tst_exit
