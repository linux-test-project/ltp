#!/bin/sh
#To run this script the following is necessary
# 1.kernel should mtd support as module.
# 2.kernel should hsve jffs2 support as module.
# 3.kernel should have loopback device support .
# 4.you should have fs-bench utility (http://h2np.net/tools/fs-bench-0.2.tar.gz)
# 5.results will be copied to /tmp/log and /tmp/log1 files

#DESCRIPTION: This testscript creates a jffs2 file system type and tests the filesystem test
#and places the log in the log directory.The file system test actually creates a tree of large
#directories and performs the delete and random delete operations as per the filesystem stress
#algorithim and gives a report of real time ,user time,system time taken to perform the file
#operations.

#script created  G.BANU PRAKASH (mailto:prakash.banu@wipro.com).
#
#   This program is free software;  you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY;  without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#   the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program;  if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

MTD_RAM=mtdram
MTD_BLOCK=mtdblock
JFFS2=jffs2
LOOP=loop
MTD_BLKDEVS=mtd_blkdevs
ZLIB_DEFLATE=zlib_deflate
ZLIB_INFLATE=zlib_inflate
MTD_CORE=mtdcore
MOUNT_DIR=/mnt
LOG_DIR=/tmp/log
LOG_DIR1=/tmp/log1
HOME_DIR=/home
BLOCK_DIR=/dev/mtdblock
export PATH=$PATH:/sbin
	if [ $(id -ru) -ne 0 ];
then
	echo "must be root to run this"
	exit
fi



lsmod |grep $MTD_RAM

	if [ $? -ne 0 ];
then
	echo "inserting mtd ram and its dependencies"
fi
modprobe $MTD_RAM  total_size=32768 erase_size=256
	if [ $? -ne 0 ];
then
	echo "check wheather MTD -mtdram is been compiled in the kernel"
fi
lsmod |grep $MTD_BLOCK
	if [ $? -ne 0 ]; then
	echo "inserting mtdblock  and its dependencies"
fi
modprobe $MTD_BLOCK
	if [ $? -ne 0 ]; then
	echo "check wheather mtdblock is been compiled in the kernel"
fi

lsmod |grep $JFFS2
	if [ $? -ne 0 ]; then
	echo "inserting jffs2  and its dependencies"
fi
modprobe $JFFS2
	if [ $? -ne 0 ]; then
	echo "check wheather jffs2 is been compiled in the kernel"
fi

lsmod |grep $LOOP
	if [ $? -ne 0 ]; then
	echo "inserting loopback device module"
fi
modprobe $LOOP
	if [ $? -ne 0 ]; then
	echo "check wheather loopback device option is been compiled in the kernel"
fi
mkdir -p $BLOCK_DIR
mknod $BLOCK_DIR/0 b 31 0 >/dev/null 2>&1
mount -t jffs2 $BLOCK_DIR/0 $MOUNT_DIR
mount|grep $JFFS2
	if [ $? -eq 0 ]; then
 echo "jffs2 mounted sucessfully"
	else
 echo "mount unsucessfull"
fi
cd $MOUNT_DIR
echo "This is will take long time "
./test.sh    >log 2>&1
./test2.sh    >log1 2>&1

mv log   $LOG_DIR
mv log1  $LOG_DIR1
cd $HOME_DIR


#cleanup
echo "unmounting $MOUNT_DIR "
umount $MOUNT_DIR
echo "removing the modules inserted"
rmmod  $MTD_BLOCK
rmmod  $MTD_BLKDEVS
rmmod  $LOOP
rmmod  $JFFS2
rmmod  $ZLIB_DEFLATE
rmmod  $ZLIB_INFLATE
rmmod  $MTD_RAM
rmmod  $MTD_CORE
rm -rf /dev/mtdblock
echo "TEST COMPLETE"
echo "RESULTS LOGGED IN FILE  /tmp/log and /tmp/log1  "
