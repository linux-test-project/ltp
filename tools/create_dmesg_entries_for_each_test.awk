#!/usr/bin/awk -f
#
#    Script for adding necessary dmesg clear/capture calls before executing
#    commands.
#
#    Copyright (C) 2012, Linux Test Project.
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
# Ngie Cooper, April 2012
#

NF && ! /^#/ {
	s = $1 "__with_dmesg_entry dmesg -c 1>/dev/null 2>&1;"
	for (i = 2; i <= NF; i++) {
		s = s " " $i
	}
	s = s "; dmesg > " DMESG_DIR "/" $1 ".dmesg.log"
	print s
}
