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
#  FILE        : autofs4.sh
#  USAGE       : autofs4.sh <disk_partition>
#
#  DESCRIPTION : A script that will test autofs on Linux system.
#  REQUIREMENTS:
#                1) System with a floppy device with a floppy disk in it.
#                2) A spare (scratch) disk partition of 100MB or larger.
#
#  HISTORY     :
#      06/11/2003 Prakash Narayana (prakashn@us.ibm.com)
#
#  CODE COVERAGE: 26.8% - fs/autofs4 (Total Coverage)
#
#                 24.1% - fs/autofs4/expire.c
#                 33.3% - fs/autofs4/init.c
#                 24.0% - fs/autofs4/inode.c
#                 29.9% - fs/autofs4/root.c
#                  0.0% - fs/autofs4/symlink.c
#                 29.1% - fs/autofs4/waitq.c
#
##############################################################


##############################################################
#
# Make sure that uid=root is running this script.
# Validate the command line argument as a block special device.
# Make sure that autofs package has been installed.
# Make sure that autofs module is built into the kernel or loaded.
#
##############################################################

if [ $UID != 0 ]
then
	echo "FAILED: Must have root access to execute this script"
	exit 1
fi

if [ $# != 1 ]
then
	echo "FAILED: Usage $0 <disk_partition>"
	exit 1
else
	disk_partition=$1
	if [ ! -b $disk_partition ]
	then
		echo "FAILED: Usage $0 <block special disk_partition>"
		exit 1
	fi
	mkfs -t ext2 $disk_partition >/dev/null 2>&1
fi

rpm -q -a | grep autofs >/dev/null 2>&1
if [ $? != 0 ]
then
	echo "FAILED: autofs package is not installed"
	exit 1
fi

grep autofs /proc/filesystems >/dev/null 2>&1
if [ $? != 0 ]
then
	echo "FAILED: autofs module is not built into the kernel or loaded"
	exit 1
fi


##############################################################
#
# Pick the floppy device name from /etc/fstab
# Format (mkfs -t ext2) the floppy to ext2 file system
# Create the /etc/auto.master
# Create the /etc/auto.media
# Create /AUTOFS directory.
#
##############################################################

floppy_dev=`grep floppy /etc/fstab | awk '{print $1}'`

if [ $floppy_dev != "" ]
then
	/sbin/mkfs -t ext2 $floppy_dev >/dev/null 2>&1
	if [ $? != 0 ]
	then
		echo "FAILED: mkfs -t ext2 $floppy_dev failed"
		exit 1
	fi
fi

if [ ! -d /AUTOFS ]
then
	mkdir -m 755 /AUTOFS
fi

echo "/AUTOFS/MEDIA	/etc/auto.media		" > /etc/auto.master
echo "floppy	-fstype=ext2					:$floppy_dev" > /etc/auto.media


##############################################################
#
# Verify that "/etc/init.d/autofs start|restart|stop|status|reload"
# command works.
#
# If fails, cleanup and exit.
#
##############################################################

/etc/init.d/autofs start >/dev/null 2>&1
if [ $? != 0 ]
then
	rm -rf /etc/auto.master /etc/auto.media /AUTOFS
	echo "FAILED: "/etc/init.d/autofs start""
	exit 1
fi
echo "Resuming test, please wait..."
sleep 15

/etc/init.d/autofs stop >/dev/null 2>&1
if [ $? != 0 ]
then
	rm -rf /etc/auto.master /etc/auto.media /AUTOFS
	echo "FAILED: "/etc/init.d/autofs stop""
	exit 1
else
	/etc/init.d/autofs start >/dev/null 2>&1
fi
sleep 15

/etc/init.d/autofs restart >/dev/null 2>&1
if [ $? != 0 ]
then
	/etc/init.d/autofs stop >/dev/null 2>&1
	rm -rf /etc/auto.master /etc/auto.media /AUTOFS
	echo "FAILED: "/etc/init.d/autofs restart""
	exit 1
fi
echo "Resuming test, please wait..."
sleep 15

/etc/init.d/autofs status >/dev/null 2>&1
if [ $? != 0 ]
then
	/etc/init.d/autofs stop >/dev/null 2>&1
	rm -rf /etc/auto.master /etc/auto.media /AUTOFS
	echo "FAILED: "/etc/init.d/autofs status""
	exit 1
fi

/etc/init.d/autofs reload >/dev/null 2>&1
if [ $? != 0 ]
then
	/etc/init.d/autofs stop >/dev/null 2>&1
	rm -rf /etc/auto.master /etc/auto.media /AUTOFS
	echo "FAILED: "/etc/init.d/autofs reload""
	exit 1
fi


##############################################################
#
# Tryout some error code paths by:
# (1) Write into automount directory
# (2) Remove automount parent directory
# (3) Automount the floppy disk
# (4) Hit automounter timeout by sleep 60; then wakeup with error
#     condition.
#
##############################################################

mkdir /AUTOFS/MEDIA/mydir >/dev/null 2>&1
rm -rf /AUTOFS >/dev/null 2>&1

mkdir /AUTOFS/MEDIA/floppy/test
cp /etc/auto.master /etc/auto.media /AUTOFS/MEDIA/floppy/test
sync; sync
echo "Resuming test, please wait..."
sleep 60
mkdir /AUTOFS/MEDIA/mydir >/dev/null 2>&1
rm -rf /AUTOFS >/dev/null 2>&1


##############################################################
#
# Add an entry to the /etc/auto.master and reload.
#
##############################################################

echo "/AUTOFS/DISK	/etc/auto.disk		" >> /etc/auto.master
echo "disk		-fstype=ext2					:$disk_partition " > /etc/auto.disk
/etc/init.d/autofs reload >/dev/null 2>&1
echo "Resuming test, please wait..."
sleep 30

mkdir /AUTOFS/DISK/disk/test
cp /etc/auto.master /etc/auto.media /AUTOFS/DISK/disk/test
sync; sync
echo "Resuming test, please wait..."
sleep 60

cd /AUTOFS/DISK/disk/test
umount /AUTOFS/DISK/disk/ >/dev/null 2>&1
if [ $? = 0 ]
then
	/etc/init.d/autofs stop >/dev/null 2>&1
	rm -rf /etc/auto.master /etc/auto.media /etc/auto.disk /AUTOFS
	echo "FAILED: unmounted a busy file system!"
	exit 1
fi
cd

umount /AUTOFS/DISK/disk/ >/dev/null 2>&1
if [ $? != 0 ]
then
	/etc/init.d/autofs stop >/dev/null 2>&1
	rm -rf /etc/auto.master /etc/auto.media /etc/auto.disk /AUTOFS
	echo "FAILED: Could not unmount automounted file system"
	exit 1
fi

#
# Mount the disk partition somewhere else and then reference automount
# point for disk partition.
#
mount -t ext2 $disk_partition /mnt/
ls -l /AUTOFS/DISK/disk
umount /mnt


#######################################################
#
# Just before exit, stop autofs and cleanup.
#
#######################################################

/etc/init.d/autofs stop >/dev/null 2>&1
rm -rf /etc/auto.master /etc/auto.media /etc/auto.disk /AUTOFS
echo "PASSED: $0 passed!"
exit 0
