#! /bin/bash

################################################################################
#                                                                              #
# Copyright (c) 2009 FUJITSU LIMITED                                           #
#                                                                              #
# This program is free software;  you can redistribute it and#or modify        #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; either version 2 of the License, or            #
# (at your option) any later version.                                          #
#                                                                              #
# This program is distributed in the hope that it will be useful, but          #
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY   #
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License     #
# for more details.                                                            #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program;  if not, write to the Free Software                 #
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA      #
#                                                                              #
################################################################################

#$1: inode version of which file
#$2: 1  - return inode version by return value
#    !1 - writting inode version to stddev

inode_version=`debugfs -R "stat $1" $EXT4_DEV 2> /dev/null | grep 'Version' | awk '{
print $NF }'`

# The inode_version's format: '0x0000000a' or '0x00000000:0000000a',
# so delete ':'
inode_version=`echo $inode_version | sed 's/://'`

inode_version=$(( $inode_version ))

if [ "$2" = "1" ]; then
	exit $inode_version
else
	echo $inode_version
fi

