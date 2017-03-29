#!/bin/sh
#
#   Copyright (c) International Business Machines  Corp., 2001
#
#   This program is free software;  you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY;  without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#   the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program;  if not, write to the Free Software
#   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
#
#   FILE: generate.sh
#
#   PURPOSE: Creates dat for use in network file transfer tests.
#
#   AUTHOR: Randy Hron (rwhron@earthlink.net)
#
############################################################################

file=$1
size=$2

if [ -z "$1" ] || [ -z "$2" ] ; then
	echo "usage: ${0##*/} <file> <size in bytes>"
	exit 1
fi

[ -e "$file" ] && exit 0

dd if=/dev/zero bs=1 count="$size" 2>/dev/null | tr "\0" "A" > "$file"

chmod 666 $file
