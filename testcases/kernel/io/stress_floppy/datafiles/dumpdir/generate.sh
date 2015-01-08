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
