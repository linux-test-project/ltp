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

if [ ! -x /usr/bin/wget ]; then
	echo "This script requires wget."
	exit 1
fi

if [ ! -x /usr/local/bin/bsign ]; then
	echo "This test will require bsign"
	exit 1
fi

/usr/bin/wget http://osdn.dl.sourceforge.net/sourceforge/disec/digsig-latest.tar.gz
tar zxf digsig-latest.tar.gz
cd digsig-latest
./digsig.init compile
