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

./$tstf
if [ $? != 0 ]; then
	echo "Error running *good* test"
fi

size=`stat -c %s $tstf`
max=$size
for count in `seq 0 $max`; do
	./swapbit $tstf 0 $count
	./$tstf
	ret=$?
	if [ $ret = 0 ]; then
		echo "Error at byte $count - return value $ret"
	fi;
	./swapbit -r $tstf 0 $count
done

exit 0
