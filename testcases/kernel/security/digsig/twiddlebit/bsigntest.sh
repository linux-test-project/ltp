#!/bin/sh

#	script created by Serge E. Hallyn <serue@us.ibm.com>
#	Copyright (c) International Business Machines  Corp., 2005
#
#	This program is free software;  you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 2 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY;  without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#	the GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program;  if not, write to the Free Software
#	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

tstf=hw
cp hw_signed $tstf
size=`stat -c %s $tstf`
echo "size is $size"

hex_offset=`readelf -S $tstf | grep signature | awk '{print $5}'`
if [ -z "$hex_offset" ]; then
    echo "No signature in file $1"
    exit 1
fi
echo "Note that hex offset of signature is $hex_offset."

bsign -V -P "--homedir=." ./$tstf
if [ $? != 0 ]; then
	echo "Error checking *good* signature"
fi

max=$size
dec_offset=0
echo "max byte is $max"
for count in `seq 0 $max`; do
	./swapbit $tstf $dec_offset $count
	bsign -V -P "--homedir=." ./$tstf
	ret=$?
	if [ $ret = 0 ]; then
		echo "Error at byte $count - return value $ret"
	fi;
	./swapbit -r $tstf $dec_offset $count
done

exit 0
