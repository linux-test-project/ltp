#!/bin/bash
#********************************************************************************#
#* 										*#
#* Copyright (c) 2005 Instituto Nokia de Tecnologia - INdT - Manaus Brazil 	*#
#* 										*#
#* This program is free software; you can redistribute it and#or modify 	*#
#* it under the terms of the GNU General Public License as published by 	*#
#* the Free Software Foundation; either version 2 of the License, or 		*#
#* (at your option) any later version. 						*#
#* 										*#
#* This program is distributed in the hope that it will be useful, 		*#
#* but WITHOUT ANY WARRANTY; without even the implied warranty of 		*#
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See 			*#
#* the GNU General Public License for more details. 				*#
#* 										*#
#* You should have received a copy of the GNU General Public License 		*#
#* along with this program; if not, write to the Free Software 			*#
#* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA 	*#
#* 										*#
#********************************************************************************#

#********************************************************************************#
#* 										*#
#* File: remove_password.sh 							*#
#* 										*#
#* Description: used to remove the password from a card previously unlocked. 	*#
#* 										*#
#* 										*#
#* Total Tests: 1 								*#
#* 										*#
#* Author: Anderson Briglia <anderson.briglia@indt.org.br> 			*#
#* Anderson Lizardo <anderson.lizardo@indt.org.br> 				*#
#* Carlos Eduardo Aguiar <carlos.aguiar@indt.org.br> 				*#
#* 										*#
#* 										*#
#* 										*#
#********************************************************************************#
remove_password()
{
	export TST_TOTAL=1  # Total number of test cases in this file.
	# Set up LTPTMP (temporary directory used by the tests).
	LTPTMP=${TMP}       # Temporary directory to create files, etc.
	export TCID="remove_password" # Test case identifier
	export TST_COUNT=0  # Set up is initialized as test 0
	RC=0                # Exit values of system commands used

	echo "=== Remove MMC Password ==="
	keyid=$(keyctl request mmc "mmc:key")
	if [ -z "$keyid" ]; then
		echo "*** No protected and unlocked MMC was found. The password cannot be removed."
		exit 1
	fi

	echo remove > /sys/bus/mmc/devices/mmc0\:*/lockable || \
	{ echo "*** Password reset failed."; exit 1 ;}

	# Clear session keyring
	# FIXME: It assumes we have only the MMC key there
	keyctl clear -3

	echo "Password removed."
}

remove_password || exit $RC
