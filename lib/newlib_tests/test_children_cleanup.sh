#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 SUSE LLC <mdoucha@suse.cz>

TMPFILE="/tmp/ltp_children_cleanup_$$.log"
./test_children_cleanup 1>$TMPFILE 2>&1
CHILD_PID=`sed -n 's/^.*Forked child \([0-9]*\)$/\1/p' "$TMPFILE"`
rm "$TMPFILE"

if [ "x$CHILD_PID" = "x" ]; then
	echo "TFAIL: Child process was not created"
	exit 1
fi

# The child process can stay alive for a short while even after receiving
# SIGKILL, especially if the system is under heavy load. Wait up to 5 seconds
# for it to fully exit.
for i in `seq 6`; do
	CHILD_STATE=`sed -ne 's/^State:\s*\([A-Z]\).*$/\1/p' "/proc/$CHILD_PID/status" 2>/dev/null`

	if [ ! -e "/proc/$CHILD_PID" ] || [ "$CHILD_STATE" = "Z" ]; then
		echo "TPASS: Child process was cleaned up"
		exit 0
	fi

	sleep 1
done

echo "TFAIL: Child process was left behind"
exit 1
