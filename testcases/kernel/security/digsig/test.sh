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

(cd digsig-latest; ./digsig.init start ../twiddlebit/test_pub_key)

echo "Running writeexec test"
cd writeexec
./libwritetest > weoutput 2>&1
cp shared.so.signed shared.so # in case people want to run multiple times

cd ..

echo "Running digsigtest"
cd twiddlebit
if [ ! -e hw ]; then
	echo "No bsign was found, not running twiddle test"
else
	./digsigtest.sh > digsigoutput 2>&1
fi
cd ..

(cd digsig-latest; ./digsig.init stop)

echo "Running bsigntest"
cd twiddlebit
if [ ! -e hw ]; then
	echo "No bsign was found, not running twiddle test"
else
	./bsigntest.sh > bsignoutput 2>&1
fi
cd ..

echo "All tests done."
