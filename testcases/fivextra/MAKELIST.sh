#!/bin/bash
###########################################################################
#  (C) Copyright IBM Corp. 2004
#
#  This program is free software;  you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY;  without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#  the GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program;  if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
# description:	Skip building named packages for given architecture,
#		Reads a file named "EXCLUDE<arch>" where "<arch>" is the
#		architecture given in the environment variable "_am_host".
#		Packages named in that file are skipped (not built).
#
#		The EXCLUDE file can contain comments (full line or
#		trailing) by starting them wihth "#".
#
# Author:	Robert Paulsen
#
# History:	28 Jun 2004	Created.
#		15 Jul 2004	Improved parsing EXCLUDE file to allow
#				comments (rcp).
#
###########################################################################

SUBDIRS=`find . -type d -maxdepth 1 ! -name "." ! -name CVS | sort`

# From the list of directories named in $SUBDIRS, keep only those NOT
# listed in the EXCLUDE file for the architecture we are building.
suffix=${mcp_host##*_}
excludefile=EXCLUDE$suffix
touch $excludefile
for d in $SUBDIRS ; do
	x=${d##*/}
	# egrep is looking for "blanks name-to-exclude blanks comment"
	# where all blanks and comment are optional.
	egrep -q "^[[:blank:]]*$x[[:blank:]]*([\#].*)*$" $excludefile ||
	echo $d
done
