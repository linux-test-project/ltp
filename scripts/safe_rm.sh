#!/bin/sh
#
# A safe wrapper around rm(1) to avoid cleaning out folks' rootfs's or build
# machines by accident.
#
#    Copyright (C) 2010, Cisco Systems Inc.
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
# Feel free to use this in your standard builds, or just leave it be (it has
# been added to the build tests that should be run before each release to avoid
# build regressions).
#
# Ngie Cooper, February 2010
#

. "${0%/*}/lib/file_functions.sh"

opts=
opts_parse_done=0

set -e

while [ $# -gt 0 ] ; do

	if [ $opts_parse_done -eq 0 ] ; then

		case "$1" in
		-*)
			[ "x$1" = "x--" ] && opts_parse_done=1
			# None of the options to rm(1) are keyed.
			opts="$opts $1"
			;;
		*)
			opts_parse_done=1
			;;
		esac

	fi

	if [ $opts_parse_done -eq 1 ] ; then

		abspath_file=$(_abspath "$1")

		if [ "x$abspath_file" = "x/" ] ; then

			cat <<EOF >&2
${0##*/}: ERROR : not removing \`$1' to avoid removing root directory\!
EOF
			false

		else

			if [ "x${SIMULATE_RM:-1}" != x1 ] ; then
				rm ${opts:--f} "$abspath_file"
			fi

		fi

	fi

	shift

done
