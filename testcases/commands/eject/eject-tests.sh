#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2001                 ##
##  Author: Manoj Iyer, manjo@mail.utexas.edu                                 ##
## Copyright (c) 2016 Cyril Hrubis <chrubis@suse.cz>                          ##
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
## along with this program;  if not, write to the Free Software Foundation,   ##
## Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA           ##
##                                                                            ##
################################################################################
#
# Tests basic functionality of eject command.
#

TST_ID="eject01"
TST_CNT=4
TST_SETUP=setup
TST_CLEANUP=cleanup
TST_TESTFUNC=test
TST_NEEDS_TMPDIR=1
TST_NEEDS_ROOT=1
TST_NEEDS_CMDS="eject"
. tst_test.sh

setup()
{
	CD_DRIVE="/dev/cdrom"

	if ! [ -e "$CD_DRIVE" ]; then
		tst_brk TCONF "There is no "$CD_DRIVE""
	fi

	if grep -q "$CD_DRIVE" /proc/mounts; then
		tst_brk TCONF "$CD_DRIVE is already mounted"
	fi

	ROD mkdir "cdrom"
}

cleanup()
{
	# We have to use the mount point since /dev/cdrom may be link to
	# /dev/sr0 and because of that /dev/cdrom is not listed in /proc/mounts
	tst_umount "$PWD/cdrom"
}

test1()
{
	EXPECT_PASS eject -d \> eject.out

	if grep -q "eject: default device:" eject.out; then
		tst_res TPASS "Eject listed default device"
	else
		tst_res TFAIL "Eject failed to list default device"
		cat eject.out
	fi
}

test2()
{
	EXPECT_PASS eject -v $CD_DRIVE \> eject.out

	if grep -q "CD-ROM eject command succeeded" eject.out; then
		# Close the tray if it is supported.
	        eject -t $CD_DRIVE > /dev/null 2>&1
		tst_res TPASS "Drive successfully ejected"
	else
		tst_res TFAIL "Eject failed"
		cat eject.out
	fi
}

mount_cdrom()
{
	local tries=100

	# Wait for the drive to spin up the disk
	while [ $tries -gt 0 ]; do
		eject_check_tray $CD_DRIVE
		if [ $? -eq 4 ]; then
			break
		fi
		tst_sleep 100ms
		tries=$((tries-1))
	done

	mount "$CD_DRIVE" cdrom/ > mount.out 2>&1
	if [ $? -eq 32 ]; then
		tst_res TCONF "Failed to mount $CD_DRIVE, no disk in drive?"
		cat mount.out
		return 0
	fi

	tst_res TINFO "$CD_DRIVE mounted sucessfully"

	return 1
}

test3()
{
	if mount_cdrom; then
		return
	fi

	test2

	if grep -q "$CD_DRIVE" /proc/mounts; then
		tst_res TFAIL "$CD_DRIVE is stil moutned"
	else
		tst_res TPASS "$CD_DRIVE umounted succesfully"
	fi
}

test4()
{
	if mount_cdrom; then
		return
	fi

	EXPECT_PASS eject -a on $CD_DRIVE

	eject_check_tray $CD_DRIVE
	if [ $? -eq 2 ]; then
		tst_brk TBROK "$CD_DRIVE is mounted but tray is open"
	fi

	EXPECT_PASS umount $CD_DRIVE

	eject_check_tray $CD_DRIVE
	if [ $? -eq 2 ]; then
		tst_res TPASS "$CD_DRIVE was auto-ejected"
	else
		tst_res TFAIL "$CD_DRIVE was not auto-ejected"
	fi

	EXPECT_PASS eject -a off $CD_DRIVE

	eject -t $CD_DRIVE > /dev/null 2>&1

	if mount_cdrom; then
		return
	fi

	EXPECT_PASS umount $CD_DRIVE

	eject_check_tray $CD_DRIVE
	if [ $? -eq 2 ]; then
		tst_res TFAIL "$CD_DRIVE was auto-ejected"
	else
		tst_res TPASS "$CD_DRIVE was not auto-ejected"
	fi
}

tst_run
