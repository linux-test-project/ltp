#!/bin/sh

COUNT=0

FILE=1K_file

for i in 10K_file 100K_file 1000K_file; do

	[ -e "$i" ] && continue

	echo "Creating \`$i'"

	COUNT=0

	while [ $COUNT -le 10 ]; do
		cat "$FILE" >> "$i"
		COUNT=$(( $COUNT + 1 ))
	done

	FILE=$i

done

for i in 1K_file 10K_file 100K_file 1000K_file; do
	diff --brief $i "dumpdir/$i" > /dev/null 2>&1 || cp "$i" dumpdir
done
