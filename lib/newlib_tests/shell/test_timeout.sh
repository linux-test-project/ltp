#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) 2019 Petr Vorel <pvorel@suse.cz>

PATH="$(dirname $0):$(dirname $0)/../../../testcases/lib/:$PATH"

DATA="
timeout01.sh||0
timeout02.sh||0
timeout02.sh|foo|2
timeout02.sh|2|0
timeout01.sh|2|0
timeout02.sh|1.1|0
timeout02.sh|-10|2
timeout02.sh|-0.1|0
timeout02.sh|-1.1|2
timeout02.sh|-10.1|2
"

echo "Testing timeout in shell API"
echo

failed=0
for i in $DATA; do
	file=$(echo $i | cut -d'|' -f1)
	timeout=$(echo $i | cut -d'|' -f2)
	exp_exit=$(echo $i | cut -d'|' -f3)

	echo "=== $file (LTP_TIMEOUT_MUL='$timeout') ==="
	LTP_TIMEOUT_MUL=$timeout $file
	ret=$?
	if [ $ret -ne $exp_exit ]; then
		echo "FAILED (exit code: $ret, expected $exp_exit)"
		failed=$((failed+1))
	else
		echo "PASSED"
	fi
	echo
done

echo "Failed tests: $failed"
exit $failed
