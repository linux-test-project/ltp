#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2022 SUSE LLC <mdoucha@suse.cz>

TMPFILE="/tmp/ltp_children_cleanup_$$.log"
./test_children_cleanup &>"$TMPFILE"
CHILD_PID=`sed -n 's/^.*Forked child \([0-9]*\)$/\1/p' "$TMPFILE"`
rm "$TMPFILE"

if [ "x$CHILD_PID" = "x" ]; then
	echo "TFAIL: Child process was not created"
	exit 1
elif ! kill -s 0 $CHILD_PID &>/dev/null; then
	echo "TPASS: Child process was cleaned up"
	exit 0
else
	echo "TFAIL: Child process was left behind"
	exit 1
fi
