#!/bin/bash


##############################################################
#
#  Copyright (c) International Business Machines  Corp., 2003
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
#  FILE        : isofs.sh
#  USAGE       : isofs.sh
#
#  DESCRIPTION : A script that will test isofs on Linux system.
#  REQUIREMENTS:
#
#  HISTORY     :
#      06/27/2003 Prakash Narayana (prakashn@us.ibm.com)
#
#  CODE COVERAGE: 40.5% - fs/isofs (Total Coverage)
#
#                 23.7% - fs/isofs/dir.c
#                 46.0% - fs/isofs/inode.c
#                 22.9% - fs/isofs/joliet.c
#                 50.0% - fs/isofs/namei.c
#                 38.5% - fs/isofs/rock.c
#                 10.7% - fs/isofs/util.c
#
##############################################################


MNT_POINT="/tmp/isofs_$$"

USAGE="$0"


##############################################################
#
# Make sure that uid=root is running this script.
# Validate the command line arguments.
#
##############################################################

if [ $UID != 0 ]
then
	echo "FAILED: Must have root access to execute this script"
	exit 1
fi

mkdir -p -m 777 $MNT_POINT


# Make ISO9660 file system with different options.
# Mount the ISO9660 file system with different mount options.

for mkisofs_opt in \
	" " \
	"-J" \
	"-hfs -D" \
	" -R " \
	"-R -J" \
	"-f -l -D -J -L -R" \
	"-allow-lowercase -allow-multidot -iso-level 3 -f -l -D -J -L -R"
do
        echo "Running mkisofs -o isofs.iso -quiet $mkisofs_opt /etc  Command"
	mkisofs -o isofs.iso -quiet $mkisofs_opt /etc 
	if [ $? != 0 ]
	then
		rm -rf isofs.iso $MNT_POINT
		echo "FAILED: mkisofs -o isofs.iso $mkisofs_opt /etc failed"
		exit 1
	fi
	for mount_opt in \
		"loop" \
		"loop,norock" \
		"loop,nojoliet" \
		"loop,block=512,unhide" \
		"loop,block=1024,cruft" \
		"loop,block=2048,nocompress" \
		"loop,check=strict,map=off,gid=bin,uid=bin" \
		"loop,check=strict,map=acorn,gid=bin,uid=bin" \
		"loop,check=relaxed,map=normal" \
		"loop,block=512,unhide,session=2"
		# "loop,sbsector=32"
	do
		echo "Running mount -o $mount_opt isofs.iso $MNT_POINT Command"
		mount -t iso9660 -o $mount_opt isofs.iso $MNT_POINT
		if [ $? != 0 ]
		then
			rm -rf isofs.iso $MNT_POINT
			echo "FAILED: mount -t iso9660 -o $mount_opt isofs.iso $MNT_POINT failed"
			exit 1
		fi
		echo "Running ls -lR $MNT_POINT Command"
		ls -lR $MNT_POINT
		exportfs -i -o no_root_squash,rw *:$MNT_POINT
		exportfs -u :$MNT_POINT
		umount $MNT_POINT
	done
	rm -rf isofs.iso
done

#######################################################
#
# Just before exit, perform the cleanup.
#
#######################################################

rm -rf $MNT_POINT
echo "PASSED: $0 passed!"
exit 0
