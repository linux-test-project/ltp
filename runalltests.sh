#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2001                 ##
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
# File:        runalltests.sh
#
# Description:  This script just calls runltp now, and is being phased out.
#		If you rely on this script for automation reasons, please
#		change your scripts soon

cd `dirname $0`
echo -e "\033[5m"
echo "*******************************************************************"
echo "*******************************************************************"
echo "**                                                               **"
echo -e "** \033[0m The runalltests.sh script is being phased out soon.  It will \033[5m**"
echo -e "** \033[0m be replaced with the scripts called 'runltp'.  If you \033[5m       **"
echo -e "** \033[0m currently rely on this script for automation scripts, you \033[5m   **"
echo -e "** \033[0m should think about changing them NOW! \033[5m                       **"
echo "**                                                               **"
echo "*******************************************************************"
echo "*******************************************************************"
echo -e "\033[0m"
for i in 1 2 3 4 5 6 7 8 9 10
do
	echo -en "\a"
	sleep 1
done

./runltp $*
