#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (c) Linux Test Project, 2021-2026

fail=0

METAPARSE="${METAPARSEDIR:-..}/metaparse"

cd "${SRCDIR:-.}"

if [ ! -x "$METAPARSE" ]; then
	echo "Error: $METAPARSE not found (wrong PATH? out-of-tree build without specifying \$METAPARSEDIR?)" >&2
	exit 1
fi

for i in *.c; do
	printf '* %s ' "$i"
	$METAPARSE $i > tmp.json
	if ! diff tmp.json $i.json >/dev/null 2>&1; then
		echo '[FAIL]'
		echo "$i output differs!"
		diff -u tmp.json $i.json
		fail=1
	else
		echo '[OK]'
	fi
done

rm -f tmp.json

exit $fail
