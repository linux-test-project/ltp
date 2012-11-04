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
#  FILE        : sysfs.sh
#  USAGE       : sysfs.sh [ -k <kernel_module> ]
#
#  DESCRIPTION : A script that will test sysfs on Linux system.
#  REQUIREMENTS: CONFIG_DUMMY must have been used to build kernel, and the
#		 dummy network module must exist.
#
#  HISTORY     :
#      06/24/2003 Prakash Narayana (prakashn@us.ibm.com)
#
#  CODE COVERAGE: 31.3% - fs/sysfs (Total Coverage)
#
#                  0.0% - fs/sysfs/bin.c
#                 61.8% - fs/sysfs/dir.c
#                 27.5% - fs/sysfs/file.c
#                 40.4% - fs/sysfs/inode.c
#                 41.2% - fs/sysfs/mount.c
#                 58.1% - fs/sysfs/symlink.c
#
##############################################################


MNT_POINT="/tmp/sysfs_$$"

KERNEL_NAME=`uname -a | awk ' { print $3 } '`
KERN_MODULE=/lib/modules/$KERNEL_NAME/kernel/drivers/net/dummy.ko
USAGE="$0 [ -k <kernel_module> ]"


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

while getopts k: args
do
	case $args in
	k)	KERN_MODULE=$OPTARG ;;
	\?)	echo $USAGE ; exit 1 ;;
	esac
done

if [ -z "$KERN_MODULE" ]
then
	echo $USAGE
	echo "FAILED: kernel module to insert not specified"
	exit 1
fi

# Here is the code coverage for fs/sysfs
# insmod/rmmod net/dummy.ko creates and deletes a directory
# under sysfs.
# In kernel, 2.5.73 or higher, insert/delete base/firmware_class.ko

mkdir -p -m 777 $MNT_POINT
mount -t sysfs sysfs $MNT_POINT
if [ $? != 0 ]
then
	echo "FAILED: sysfs mount failed"
	exit 1
fi

insmod $KERN_MODULE
if [ $? != 0 ]
then
	umount $MNT_POINT
	rm -rf $MNT_POINT
	echo "FAILED: insmod failed"
	exit 1
fi

rmmod $KERN_MODULE
if [ $? != 0 ]
then
	umount $MNT_POINT
	rm -rf $MNT_POINT
	echo "FAILED: rmmod failed"
	exit 1
fi


#######################################################
#
# Just before exit, perform the cleanup.
#
#######################################################

umount $MNT_POINT
rm -rf $MNT_POINT

echo "PASSED: $0 passed!"
exit 0
