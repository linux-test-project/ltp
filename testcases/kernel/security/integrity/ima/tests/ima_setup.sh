#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (C) 2009 IBM Corporation                                         ##
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
# File :        ima_setup.sh
#
# Description:  setup/cleanup routines for the integrity tests.
#
# Author:       Mimi Zohar, zohar@ibm.vnet.ibm.com
################################################################################
. test.sh
mount_sysfs()
{
	SYSFS=$(mount 2>/dev/null | awk '$5 == "sysfs" { print $3 }')
	if [ "x$SYSFS" = x ] ; then

		SYSFS=/sys

		test -d $SYSFS || mkdir -p $SYSFS 2>/dev/null
		if [ $? -ne 0 ] ; then
			tst_brkm TBROK "Failed to mkdir $SYSFS"
		fi
		if ! mount -t sysfs sysfs $SYSFS 2>/dev/null ; then
			tst_brkm TBROK "Failed to mount $SYSFS"
		fi

	fi
}

mount_securityfs()
{
	SECURITYFS=$(mount 2>/dev/null | awk '$5 == "securityfs" { print $3 }')
	if [ "x$SECURITYFS" = x ] ; then

		SECURITYFS="$SYSFS/kernel/security"

		test -d $SECURITYFS || mkdir -p $SECURITYFS 2>/dev/null
		if [ $? -ne 0 ] ; then
			tst_brkm TBROK "Failed to mkdir $SECURITYFS"
		fi
		if ! mount -t securityfs securityfs $SECURITYFS 2>/dev/null ; then
			tst_brkm TBROK "Failed to mount $SECURITYFS"
		fi

	fi
}

setup()
{
	tst_require_root

	tst_tmpdir

	mount_sysfs

	# mount securityfs if it is not already mounted
	mount_securityfs

	# IMA must be configured in the kernel
	IMA_DIR=$SECURITYFS/ima
	if [ ! -d "$IMA_DIR" ]; then
		tst_brkm TCONF "IMA not enabled in kernel"
	fi
}

cleanup()
{
	tst_rmdir
}
