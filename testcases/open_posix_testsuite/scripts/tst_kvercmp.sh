#!/bin/bash

if [ $# -ne 3 ]; then
	echo "Usage: ./tst_kvercmp.sh r1 r2 r3"
	exit 1
fi

ker_ver=$(uname -r)
r1=$(echo ${ker_ver} | awk -F. '{print $1}')
r2=$(echo ${ker_ver} | awk -F. '{print $2}')
r3=$(echo ${ker_ver} | awk -F. '{print $3}')
r3=${r3%%-*}
r3=${r3%%+*}

test_ver=$(($1 * 65536 + $2 * 256 + $3))
curr_ver=$((${r1} * 65536 + ${r2} * 256 + ${r3}))
if [ ${curr_ver} -ge ${test_ver} ]; then
	echo 0
else
	echo 1
fi
