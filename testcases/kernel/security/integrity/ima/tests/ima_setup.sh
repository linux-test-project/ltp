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
	SYSFS=`mount | grep sysfs` || RC=$?
	if [ $RC -eq 1 ]; then
		SYSFS=/sys
		mkdir -p $SYSFS
		`mount -t sysfs sysfs $SYSFS`
		RC=$?
		return $RC
	else
		SYSFS=`echo $SYSFS |  sed 's/sysfs on //' | sed 's/ type .*//'`
	fi
	return 0
}

mount_securityfs()
{
	SECURITYFS=`mount | grep securityfs` || RC=$?
	if [ $RC == 1 ]; then
		SECURITYFS=$SYSFS/kernel/security
		`mkdir -p $SECURITYFS`
		`mount -t securityfs securityfs $SECURITYFS`
		RC=$?
		return $RC
	else
		SECURITYFS=`echo $SECURITYFS |  sed 's/securityfs on //' \
			| sed 's/ type .*//'`
	fi
	return 0
}

setup()
{
	export TST_TOTAL=1
	export TCID="setup"
        export TST_COUNT=0

	trap "cleanup" 0
	if [ -z $TMP ]; then
		LTPTMP=/tmp
	else
		LTPTMP=${TMP}
	fi
	if [ -z $LTPBIN ]; then
		LTPBIN=../../../../../bin
		PATH=$PATH:$LTPBIN
	fi

	# Must be root
	if [ $UID -ne 0 ]; then
		tst_brkm TBROK $LTPTMP/imalog.$$ \
		 "$TCID: Must be root to execute test"
		return 1
	fi

	if [ -z $TMP ]; then
		LTPTMP=/tmp
	else
		LTPTMP=${TMP}
	fi

	# create the temporary directory used by this testcase
	LTPIMA=$LTPTMP/ima
	umask 077
	mkdir $LTPIMA &>/dev/null || RC=$?
	if [ $RC -ne 0 ]; then
		tst_brk TBROK "$TCID: Unable to create temporary directory"
		return $RC
	fi

	# mount sysfs if it is not already mounted
	mount_sysfs || RC=$?
	if [ $RC -ne 0 ]; then
		tst_brkm TBROK $LTPTMP/imalog.$$ "$TCID: cannot mount sysfs"
		return $RC
	fi

	# mount securityfs if it is not already mounted
	mount_securityfs || RC=$?
	if [ $RC -ne 0 ]; then
		tst_brkm TBROK $LTPTMP/imalog.$$ "$TCID: cannot mount securityfs"
		return $RC
	fi

	SECURITYFS=`echo $SECURITYFS |  sed 's/securityfs on //' \
		| sed 's/ type .*//'`

	# IMA must be configured in the kernel
	IMA_DIR=$SECURITYFS/ima
	if [ ! -d $IMA_DIR ]; then
		tst_brkm TBROK $LTPTMP/imalog.$$\
		 "INIT: IMA not enabled in kernel"
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
	rm -rf $LTPIMA || RC $?
	return $RC
}
