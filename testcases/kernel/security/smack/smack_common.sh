#!/bin/sh
#
#    testcases/security/smack/smack_common.sh
#
#    Copyright (C) 2009, Cisco Systems Inc.
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License along
#    with this program; if not, write to the Free Software Foundation, Inc.,
#    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Ngie Cooper, July 2009
#
# This file serves the sole purpose of executing every common piece of
# prerequisite code for all of the smack tests, s.t. a lot of duplicate shell
# code isn't laying around all over the place.
#

smackfsdir=${smackfsdir:=/smack}

check_mounted()
{
	grep -q $smackfsdir /proc/mounts
	if [ $? -ne 0 ]; then
		tst_brkm TCONF "smackfs not mounted at \"$smackfsdir\""
	fi
}

check_onlycap()
{
	onlycap=$(cat "$smackfsdir/onlycap" 2>/dev/null)
	if [ -n "$onlycap" ]; then
		tst_brkm TCONF "\"$smackfsdir/onlycap\" is \"$onlycap\", not" \
			       "the expected \"\"."
	fi
}

check_mounted
check_onlycap
