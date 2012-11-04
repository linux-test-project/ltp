#!/bin/bash
#********************************************************************************#
#*						 				*#
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
#* File: force_erase.sh 							*#
#* 										*#
#* Description: used to force-erase a card, usually when the user has forgot	*#
#* the password and wants to unlock the card. NOTE: all the card's contents are *#
#* lost when using this option! It only works for _locked_ cards.		*#
#*										*#
#* Total Tests: 1 								*#
#* 										*#
#* Author: Anderson Briglia <anderson.briglia@indt.org.br> 			*#
#* Anderson Lizardo <anderson.lizardo@indt.org.br> 				*#
#* Carlos Eduardo Aguiar <carlos.aguiar@indt.org.br> 				*#
#* 										*#
#* 										*#
#* 										*#
#********************************************************************************#
force_erase()
{
	export TST_TOTAL=1  # Total number of test cases in this file.
	# Set up LTPTMP (temporary directory used by the tests).
	LTPTMP=${TMP}       # Temporary directory to create files, etc.
	export TCID="force_erase" # Test case identifier
	export TST_COUNT=0  # Set up is initialized as test 0
	RC=0                # Exit values of system commands used

	echo "=== Erase MMC Password (AND its contents!) ==="
	if grep -q "unlocked" /sys/bus/mmc/devices/mmc0\:*/lockable; then
		echo -n "*** No locked MMC card was found. You can only use the forced "
		echo "erase operation on locked cards."
		exit 1
	fi

	while [ -z "$yn" ]; do
		read -p "WARNING: all card contents will be lost! Continue? (y/n) " yn
		case "$yn" in
			y|Y) break ;;
			n|N) exit 0 ;;
			*) yn=""
		esac
	done

	echo "Erasing card. This may take some time, wait..."
	echo erase > /sys/bus/mmc/devices/mmc0\:*/lockable || \
	{ echo "*** Error erasing card" >&2; exit 1 ;}

	# Clear session keyring
	# FIXME: It assumes we have only the MMC key there
	keyctl clear -3

	echo "Card unlocked and erased."
}

force_erase || exit $RC
