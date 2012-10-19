#!/bin/sh
#To exectute this you need mongo filesystem utility.
#Run this inside the mongo directory.
#mongo utility can be found in www.namesys.com/benchmarks/mongo-xxx.tgz
#Description-this script tests the mongo utility which actulally give the time ie cpu time
#Real time etc on reiserfile system and jfs filesystem.
#created by prakash.banu@wipro.com
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

LOG_DIR=$PWD
TEST_DIR=testdir


   		#should be root  to execute this script .
	if [ $(id -ru) -ne 0 ]; then
		echo "This script must be run as root"
		exit
	fi
		#set the PATH variable if its not done .
export PATH=$PATH:/sbin
lsmod |grep reiserfs

	if [ $? -ne 0 ]; then
		echo "inserting reiserfs and its dependencies"
	fi
modprobe reiserfs
	if [ $? -ne 0 ]; then
		echo "check wheather reiserfs  is been compiled in the kernel"
	fi

lsmod |grep loop
	if [ $? -ne 0 ]; then
		echo "inserting loopback device module"
	fi
modprobe loop
	if [ $? -ne 0 ]; then
		echo "check wheather loopback device option is been compiled in the kernel"
	fi

	#run the mongo test on reiserfs file system type
reiserfs()
{
cat > fs.sh <<EOF
echo "performing mongo on reiserfs"
dd if=/dev/zero of=reiserfs  bs=8k count=10240 > /dev/null 2>&1
losetup /dev/loop0 reiserfs
mkdir -p $TEST_DIR
./mongo.pl LOG=/tmp/logfile1 file_size=10000 bytes=100000 fstype=reiserfs dev=/dev/loop0 dir=$TEST_DIR RUN   log=$LOG_DIR/reiserlog > /dev/null 2>&1

echo "RESULTS LOGGED IN $LOG_DIR/reiserlog"
export PATH=$PATH:/sbin
losetup -d /dev/loop0

EOF
}


#To run on jfs file system type
JFS()
{
cat >> fs.sh <<EOF
echo "performing mongo on jfs file system"
mkdir -p $TEST_DIR
dd if=/dev/zero of=jfs  bs=8k count=10240 > /dev/null 2>&1
losetup /dev/loop0 jfs
./mongo.pl LOG=/tmp/logfile1 file_size=10000 bytes=100000 fstype=jfs dev=/dev/loop0 dir=$TEST_DIR   RUN log=$LOG_DIR/jfslog

echo "RESULTS LOGGED IN $LOG_DIR/jfslog"
export PATH=$PATH:/sbin
losetup -d /dev/loop0
echo "rm -rf ./fs.sh" >> ./fs.sh 2>&1
EOF
}


echo -ne "TEST ON REISERFS?[y/N]:"
read ker

case $ker in
y|Y)     reiserfs
esac

echo -ne "TEST ON JFS[y/N]: "
read ker

case $ker in
y|Y)     JFS
esac

echo "THIS MAY TAKE SOME MINUTES"
sh fs.sh

#performing cleanup
#losetup -d /dev/loop0
rm -rf $TEST_DIR
