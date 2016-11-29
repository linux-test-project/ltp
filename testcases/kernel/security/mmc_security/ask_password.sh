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
#* File: ask_password.sh 							*#
#* 										*#
#* Description: get the password from userspace. It's called when unlocking 	*#
#* the card or assigning a new password to an unlocked card. 			*#
#* Return        - zero on success						*#
#*               - non zero on failure. return value from commands ($RC)	*#
#* Total Tests: 1 								*#
#*										*#
#* Author: Anderson Briglia <anderson.briglia@indt.org.br> 			*#
#* Anderson Lizardo <anderson.lizardo@indt.org.br> 				*#
#* Carlos Eduardo Aguiar <carlos.aguiar@indt.org.br> 				*#
#* 										*#
#* 										*#
#* 										*#
#********************************************************************************#

ask_password()
{
	export TST_TOTAL=1  # Total number of test cases in this file.
	# Set up LTPTMP (temporary directory used by the tests).
	LTPTMP=${TMP}       # Temporary directory to create files, etc.
	export TCID="ask_password" # Test case identifier
	export TST_COUNT=0  # Set up is initialized as test 0
	RC=0                # Exit values of system commands used

	USER_CONSOLE=/dev/ttyS0
	{
		echo "=== Unlock Protected MMC ==="
		while [ -z "$passwd" ]; do
			read -s -p "MMC password: " passwd; echo
		done
		if ! keyctl instantiate $1 "$passwd" $2 >/dev/null 2>&1; then
			echo "*** Wrong password! The card was not unlocked."
			exit 1
		fi
		echo "Password accepted."

		exit 0
	} >$USER_CONSOLE 2>&1 < $USER_CONSOLE
}

ask_password || exit $RC
