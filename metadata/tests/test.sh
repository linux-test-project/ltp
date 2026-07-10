#!/bin/sh

fail=0

for i in *.c; do
	printf '* %s ' "$i"
	../metaparse $i > tmp.json
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
