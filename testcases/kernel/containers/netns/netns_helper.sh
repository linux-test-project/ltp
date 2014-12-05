#!/bin/sh
#
# Copyright (c) Linux Test Project, 2014
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# This is a LTP shell test library for iproute.
#

. test.sh

tst_check_iproute()
{
	local cur_ipver=`ip -V`
	local spe_ipver=$1

	cur_ipver=${cur_ipver##*s}

	if [ -z $cur_ipver ] || [ -z $spe_ipver ]; then
		tst_brkm TBROK "don't obtain valid iproute version"
	fi

	if [ $cur_ipver -lt $spe_ipver ]; then
		tst_brkm TCONF \
			"The commands in iproute tools do not support required objects"
	fi
}
