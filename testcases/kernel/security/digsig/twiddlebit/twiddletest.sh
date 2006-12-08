#!/bin/sh

tstf=hw
#gcc -o $tstf $tstf.c
#gcc -o swapbit swapbit.c
#/usr/local/bin/bsign -s -P "--homedir=." $tstf --force-resign

hex_offset=`readelf -S $tstf | grep signature | awk '{print $5}'`

if [ -z "$hex_offset" ]; then
    echo "No signature in file $1"
    exit 1
fi

# convert hex offset in dec offset, and copy with dd
temp=`echo "$hex_offset" | tr '[a-f]' '[A-F]'`
dec_offset=`echo "ibase=16; $temp" | bc`

./$tstf
if [ $? != 0 ]; then
	echo "Error running *good* test"
fi

for count in `seq 0 511`; do
	./swapbit $tstf $dec_offset $count
	./$tstf
	ret=$?
	if [ $ret = 0 ]; then
		echo "Error at bit $count - return value $ret"
#		exit 1
	fi;
	./swapbit -r $tstf $dec_offset $count
done

exit 0
