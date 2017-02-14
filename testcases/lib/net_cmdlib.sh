#!/bin/sh
#
#    An extension to cmdlib.sh, purely for networking based commands.
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
# Ngie Cooper, August 2009
#

. cmdlib.sh

# Blank for an IPV4 test; 6 for an IPV6 test.
EXEC_SUFFIX=

read_opts()
{
    while getopts "6" opt; do
        case "$opt" in
        6)
            EXEC_SUFFIX=6;;
        *)
            echo "Setup  0  : FAIL Unknown option: $opt"
            exit 1;;
        esac
    done
}
