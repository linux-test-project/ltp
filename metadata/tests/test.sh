#!/bin/sh

fail=0

for i in *.c; do
	../metaparse $i > tmp.json
	if ! diff tmp.json $i.json &> /dev/null; then
		echo "***"
		echo "$i output differs!"
		diff -u tmp.json $i.json
		echo "***"
		fail=1
	fi
done

rm -f tmp.json

exit $fail
