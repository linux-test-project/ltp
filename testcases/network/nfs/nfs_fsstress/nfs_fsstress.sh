#!/bin/sh
#
#   Copyright (c) International Business Machines  Corp., 2003
#
#   This program is free software;  you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY;  without even the implie; warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#   the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program;  if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
#
#
#  FILE   : nfs_fsstress.sh
#
#  PURPOSE: Runs fsstress over an NFS mount point for a specified amount
#          of time.  The test will also start 'sar' to monitor the system
#	   utilization data.  The purpose of this test is to stress the
#	   NFS kernel code and possibly the underlying filesystem where
#	   the export resides.  A PASS is if the test completes.
#
#  PREREQUISITE: There must exist an NFS exported filesystem available to 
#		test on.
#
#
#  HISTORY:
#    11/21/03 Robbie Williamson (robbiew@us.ibm.com)
#      -Written
#
#***********************************************************************

#Uncomment line below for debug output.
#trace_logic=${trace_logic:-"set -x"}

$trace_logic

echo -n "Please enter the server location of the NFS exported filesystem: "
read RHOST
echo -n "Please enter the name of the NFS exported filesystem: "
read FILESYSTEM
echo -n "Please enter the test duration in hours: "
read DURATION

HOSTNAME=$(hostname | awk {'print $1'})

#Convert to seconds
DURATION=$(( $DURATION * 60 * 60 ))

echo "Setting up local mount points"
mkdir -p /mnt/udp/2/${HOSTNAME}1
mkdir -p /mnt/udp/3/${HOSTNAME}2
mkdir -p /mnt/tcp/2/${HOSTNAME}3
mkdir -p /mnt/tcp/3/${HOSTNAME}4

echo "Mounting NFS filesystem"
mount -o "proto=udp,vers=2" $RHOST:$FILESYSTEM /mnt/udp/2/${HOSTNAME}1
mount -o "proto=tcp,vers=2" $RHOST:$FILESYSTEM /mnt/tcp/2/${HOSTNAME}2
mount -o "proto=udp,vers=3" $RHOST:$FILESYSTEM /mnt/udp/3/${HOSTNAME}3
mount -o "proto=tcp,vers=3" $RHOST:$FILESYSTEM /mnt/tcp/3/${HOSTNAME}4

echo "Test set for $DURATION seconds. Starting test processes now."
./fsstress -l 0 -d /mnt/udp/2/${HOSTNAME}1 -n 1000 -p 50 -r > /tmp/nfs_fsstress.udp.2.log 2>&1 &
./fsstress -l 0 -d /mnt/udp/3/${HOSTNAME}2 -n 1000 -p 50 -r > /tmp/nfs_fsstress.udp.3.log 2>&1 &
./fsstress -l 0 -d /mnt/tcp/2/${HOSTNAME}3 -n 1000 -p 50 -r > /tmp/nfs_fsstress.tcp.2.log 2>&1 &
./fsstress -l 0 -d /mnt/tcp/3/${HOSTNAME}4 -n 1000 -p 50 -r > /tmp/nfs_fsstress.tcp.3.log 2>&1 &

echo "Starting sar"
sar -o /tmp/nfs_fsstress.sardata 30 0 &

echo "Testing in progress"
sleep $DURATION
echo -n "Testing done. Killing processes"
killall -9 sadc
killall -9 fsstress
ps -ef | grep -v grep | grep fsstress > /dev/null 2>&1
TESTING=$?
while [ $TESTING -eq 0 ]
do
  killall -9 fsstress
  echo -n "."
  sleep 5
  ps -ef | grep -v grep | grep fsstress > /dev/null 2>&1
  TESTING=$?
done 
echo " "
echo "All processes killed."
echo "Unmounting NFS filesystem"
umount /mnt/udp/2/${HOSTNAME}1
umount /mnt/udp/3/${HOSTNAME}2
umount /mnt/tcp/2/${HOSTNAME}3
umount /mnt/tcp/3/${HOSTNAME}4
echo " Testing Complete: PASS"


