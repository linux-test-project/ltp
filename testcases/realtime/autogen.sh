################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2007                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################

#!/bin/bash

#
# usage:
#
# banner <target name>
#
banner() {
	echo
	TG=`echo $1 | sed -e "s,/.*/,,g"`
	LINE=`echo $TG |sed -e "s/./-/g"`
	echo $LINE
	echo $TG
	echo $LINE
	echo
}


ACLOCAL=${ACLOCAL:=aclocal}
AUTOHEADER=${AUTOHEADER:=autoheader}
AUTOMAKE=${AUTOMAKE:=automake}
AUTOCONF=${AUTOCONF:=autoconf}

#$ACLOCAL --version | \
#   awk -vPROG="aclocal" -vVERS=1.7\
#   '{if ($1 == PROG) {gsub ("-.*","",$4); if ($4 < VERS) print PROG" < version "VERS"\nThis may result in errors\n"}}'

#$AUTOMAKE --version | \
#   awk -vPROG="automake" -vVERS=1.7\
#   '{if ($1 == PROG) {gsub ("-.*","",$4); if ($4 < VERS) print PROG" < version "VERS"\nThis may result in errors\n"}}'


#banner "running libtoolize"
#libtoolize --force || exit

banner "running aclocal"
$ACLOCAL -I config/m4 || exit

banner "running autoheader"
$AUTOHEADER || exit

banner "running automake"
$AUTOMAKE --gnu --add-missing -Wall || exit

banner "running autoconf"
$AUTOCONF -Wall || exit

banner "Finished"
