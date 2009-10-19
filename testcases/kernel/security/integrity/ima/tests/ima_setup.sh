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
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :        ima_setup.sh
#
# Description:  setup/cleanup routines for the integrity tests.
#
# Author:       Mimi Zohar, zohar@ibm.vnet.ibm.com
#
# Return        - zero on success
#               - non zero on failure. return value from commands ($RC)
################################################################################
mount_sysfs()
{

	SYSFS=$(mount 2>/dev/null | awk '$5 == "sysfs" { print $3 }')
	if [ "x$SYSFS" = x ] ; then

		SYSFS=/sys

		test -d $SYSFS || mkdir -p $SYSFS 2>/dev/null
		if [ $? -ne 0 ] ; then
			tst_resm TBROK "Failed to mkdir $SYSFS"
			return 1
		fi
		if ! mount -t sysfs sysfs $SYSFS 2>/dev/null ; then
			tst_resm TBROK "Failed to mount $SYSFS"
			return 1
		fi

	fi

	return 0
}

mount_securityfs()
{

	SECURITYFS=$(mount 2>/dev/null | awk '$5 == "securityfs" { print $3 }')
	if [ "x$SECURITYFS" = x ] ; then

		SECURITYFS="$SYSFS/kernel/security"

		test -d $SECURITYFS || mkdir -p $SECURITYFS 2>/dev/null
		if [ $? -ne 0 ] ; then
			tst_resm TBROK "Failed to mkdir $SECURITYFS"
			return 1
		fi
		if ! mount -t securityfs securityfs $SECURITYFS 2>/dev/null ; then
			tst_resm TBROK "Failed to mount $SECURITYFS"
			return 1
		fi

	fi

	return 0

}

setup()
{
	export TST_TOTAL=1
	export TCID="setup"
        export TST_COUNT=0

	LTPBIN=
	LTPIMA=

	trap "cleanup" 0
	if [ -z "$TMPDIR" ]; then
		LTPTMP=/tmp
	else
		LTPTMP=${TMPDIR}
	fi
	if [ -z "$LTPBIN" ]; then
		LTPBIN=../../../../../bin
		PATH=$PATH:$LTPBIN
	fi

	# Must be root
	if ! is_root; then
		tst_resm TCONF "You must be root to execute this test"
		return 1
	fi

	# create the temporary directory used by this testcase
	LTPIMA=$LTPTMP/ima
	umask 077
	mkdir $LTPIMA > /dev/null 2>&1 || RC=$?
	if [ $RC -ne 0 ]; then
		tst_resm TBROK "Unable to create temporary directory"
		return $RC
	fi

	# mount sysfs if it is not already mounted
	mount_sysfs || RC=$?
	if [ $RC -ne 0 ]; then
		tst_resm TBROK "Cannot mount sysfs"
		return $RC
	fi

	# mount securityfs if it is not already mounted
	mount_securityfs || RC=$?
	if [ $RC -ne 0 ]; then
		tst_resm TBROK "Cannot mount securityfs"
		return $RC
	fi

	mount

	# IMA must be configured in the kernel
	IMA_DIR=$SECURITYFS/ima
	if [ ! -d "$IMA_DIR" ]; then
		tst_resm TCONF "IMA not enabled in kernel"
		RC=1
	fi
	return $RC
}

# Function:     cleanup
#
# Description   - remove temporary files and directories.
#
# Return        - zero on success
#               - non zero on failure. return value from commands ($RC)
cleanup()
{
	tst_resm TINFO "CLEAN: removing $LTPIMA"
	rm -rf "$LTPIMA" || RC=$?
	return $RC
}

. cmdlib.sh
