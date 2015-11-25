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
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
#  FILE        : exportfs.sh
#  USAGE       : exportfs.sh -h <nfs_server> -d <nfs_server_disk_partition>
#                            -t <server_fs_type>
#
#  DESCRIPTION : A script that will test exportfs on Linux system.
#  REQUIREMENTS:
#                1) NFS Server system with rsh enabled between client & server.
#                2) 100MB Disk partition on NFS server.
#
#  HISTORY     :
#      06/18/2003 Prakash Narayana (prakashn@us.ibm.com)
#
#  CODE COVERAGE: 7.1% - fs/exportfs (Total Coverage)
#                 7.1% - fs/exportfs/expfs.c
#
##############################################################


NFS_SERVER=""
REM_DISK_PART=""
FS_TYPE=""
MNT_POINT="/tmp/exportfs_$$"

USAGE="$0 -h <nfs_server> -d <nfs_server_disk_partition> -t <server_fs_type>"


##############################################################
#
# Make sure that uid=root is running this script.
# Validate the command line arguments.
# Make sure that NFS Server is up with rsh is enabled.
# Make sure that FS_TYPE package has been installed on NFS Server side.
# Make sure that FS_TYPE module is built into the kernel or loaded
# on NFS Server side.
#
##############################################################

if [ $UID != 0 ]
then
	echo "FAILED: Must have root access to execute this script"
	exit 1
fi

while getopts h:d:t: args
do
	case $args in
	h)	NFS_SERVER=$OPTARG ;;
	d)	REM_DISK_PART=$OPTARG ;;
	t)	FS_TYPE=$OPTARG ;;
	\?)	echo $USAGE ; exit 1 ;;
	esac
done

if [ -z "$NFS_SERVER" ]
then
	echo $USAGE
	echo "FAILED: NFS Server not specificed"
	exit 1
fi

if [ -z "$REM_DISK_PART" ]
then
	echo $USAGE
	echo "FAILED: NFS Server disk partition not specified"
	exit 1
fi

if [ -z "$FS_TYPE" ]
then
	echo $USAGE
	echo "FAILED: NFS Server file system type not specified"
	exit 1
fi

#
# How to check if it a valid block special device on NFS Server ???
# Add code here.


ping -c 2 -w 15 $NFS_SERVER >/dev/null 2>&1
if [ $? != 0 ]
then
	echo "FAILED: ping $NFS_SERVER failed"
	exit 1
fi

rsh -n -l root $NFS_SERVER "ls -l /etc" >/dev/null 2>&1
if [ $? != 0 ]
then
	echo "FAILED: rsh -n -l root $NFS_SERVER "ls -l /etc" failed"
	exit 1
fi

rsh -n -l root $NFS_SERVER "rpm -q -a | grep $FS_TYPE" | grep $FS_TYPE >/dev/null 2>&1
if [ $? != 0 ]
then
	rsh -n -l root $NFS_SERVER "grep $FS_TYPE /etc/filesystems" | grep $FS_TYPE >/dev/null 2>&1
	if [ $? != 0 ]
	then
		rsh -n -l root $NFS_SERVER "grep $FS_TYPE /proc/filesystems" | grep $FS_TYPE >/dev/null 2>&1
		if [ $? != 0 ]
		then
			echo "FAILED: $FS_TYPE package is not installed or loaded on $NFS_SERVER"
			exit 1
		fi
	fi
fi

if [ "$FS_TYPE" = "reiserfs" ]
then
#	rsh -n -l root $NFS_SERVER "/sbin/mkfs -t $FS_TYPE --format 3.6 -f $REM_DISK_PART >/dev/null 2>&1"
	rsh -n -l root $NFS_SERVER "/sbin/mkfs -t $FS_TYPE -f $REM_DISK_PART --format 3.6 >/dev/null 2>&1"
	echo "/sbin/mkfs -t $FS_TYPE --format 3.6 -f $REM_DISK_PART >/dev/null 2>&1"
else
#	rsh -n -l root $NFS_SERVER "/sbin/mkfs -t $FS_TYPE $REM_DISK_PART >/dev/null 2>&1"
	QUIETFLAG=
	if [ "$FS_TYPE" = "jfs" ]
	then
		QUIETFLAG="-q"
	fi
	rsh -n -l root $NFS_SERVER "/sbin/mkfs -t $FS_TYPE $QUIETFLAG $REM_DISK_PART >/dev/null 2>&1"
	if [ $? != 0 ]
	then
		echo "FAILED: Could not /sbin/mkfs -t $FS_TYPE $REM_DISK_PART on $NFS_SERVER"
		exit 1
	fi
fi

rsh -n -l root $NFS_SERVER "mkdir -p -m 777 $MNT_POINT"
if [ $? != 0 ]
then
	echo "FAILED: Could not mkdir -p -m 777 $MNT_POINT on $NFS_SERVER"
	exit 1
fi

rsh -n -l root $NFS_SERVER "mount -t $FS_TYPE $REM_DISK_PART $MNT_POINT"
if [ $? != 0 ]
then
	echo "FAILED: Could not mount -t $FS_TYPE $REM_DISK_PART on $MNT_POINT"
	exit 1
fi

rsh -n -l root $NFS_SERVER "chmod 777 $MNT_POINT"
if [ $? != 0 ]
then
	echo "FAILED: Could not chmod 777 $MNT_POINT on $NFS_SERVER"
	exit 1
fi

rsh -n -l root $NFS_SERVER "/usr/sbin/exportfs -i -o no_root_squash,rw *:$MNT_POINT"
if [ $? != 0 ]
then
	rsh -n -l root $NFS_SERVER "umount $MNT_POINT"
	rsh -n -l root $NFS_SERVER "rm -rf $MNT_POINT"
	echo "FAILED: Could not export remote directory $MNT_POINT"
	exit 1
fi
sleep 15

# Here is the code coverage for fs/exportfs
#
mkdir -p -m 777 $MNT_POINT
mount -t nfs $NFS_SERVER:$MNT_POINT $MNT_POINT
if [ $? != 0 ]
then
	echo "FAILED: NFS mount failed"
	exit 1
fi

mkdir -p -m 777 $MNT_POINT/test_dir
echo "NFS mount of $FS_TYPE file system and I/O to NFS mount point generates the  fs/exportfs code coverage" > $MNT_POINT/test_dir/exportfs_coverage


#######################################################
#
# Just before exit, perform NFS CLIENT & SERVER cleanup
#
#######################################################

umount $MNT_POINT
rm -rf $MNT_POINT

rsh -n -l root $NFS_SERVER "/usr/sbin/exportfs -u :$MNT_POINT"
rsh -n -l root $NFS_SERVER "umount $MNT_POINT"
rsh -n -l root $NFS_SERVER "rm -rf $MNT_POINT"
echo "PASSED: $0 passed!"
exit 0
