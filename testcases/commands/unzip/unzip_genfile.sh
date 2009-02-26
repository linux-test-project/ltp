#! /bin/sh
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
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File:			unzip_genfile.sh
#
# Description:	This program will generate the zip file that will be used to
#               test the unzip program. 
# 
# Author:		Manoj Iyer manjo@mail.utexas.edu
#
# History:
# 	Mar 03 2003 - Created - Manoj Iyer.

# Create directories and fill them with files.

numdirs=3                     # number of directories to create
numfiles=3                    # number of file to create in each directory
dirname=$1		      # name of the base directory
dircnt=0                      # index into number of dirs created in loop
fcnt=0                        # index into number of files created in loop
RC=0                          # return value from commands

while [ $dircnt -lt $numdirs ]
do
	dirname=$dirname/d.$dircnt
	mkdir -p $dirname || RC=$?
	if [ $RC -ne 0 ]
	then
		echo "$0: ERROR: while creating $numdirs dirs." 1>&2
		exit $RC
	fi
	fcnt=0
	while [ $fcnt -lt $numfiles ]
	do
		touch $dirname/f.$fcnt
		if [ $RC -ne 0 ]
		then
			echo "$0: ERROR: creating $numdirs dirs." 1>&2
			exit $RC
		fi
		fcnt=$(($fcnt+1))
	done
	dircnt=$(($dircnt+1))
done
