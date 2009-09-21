#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2009                 ##
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

LTPTMP=/tmp/p9auth_ltp

TOUCH=`which touch`
ID=`which id`
echo ltptmp is $LTPTMP

myuid=`id -u`
if [ "$myuid" -eq 0 ]; then
	echo "Unprivileged child was started as root!"
	exit 1
fi

$TOUCH $LTPTMP/d/childready

while [ 1 ]; do
	if [ -f $LTPTMP/childexit ]; then
		exit 0
	fi
	if [ -f $LTPTMP/childgo ]; then
		echo -n `cat $LTPTMP/d/txtfile` > /dev/capuse
		if [ `$ID -u` -eq 0 ]; then
			$TOUCH $LTPTMP/d/childpass
		else
			$TOUCH $LTPTMP/d/childfail
		fi
		exit 0
	fi
done

exit 0
