#!/bin/sh

echo "Attempting to parse currently running kernel config"
./test_kconfig
echo

for i in config0*; do
	head -n 1 "$i"
	KCONFIG_PATH="$i" ./test_kconfig
	echo
done
