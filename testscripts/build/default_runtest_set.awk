#!/usr/bin/awk -f
#
#    Script for determining default runtest set from runltp / runltplite.sh.
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
# Invoke this awk script like:
#
# $0 /path/to/ltp/runltp
#
# e.g.
#
# $0 /opt/ltp/runltp
#
# NOTE: $0 corresponds to the script name.
#

BEGIN { RS="\n"; FS=" "; }
# We only care about lines with \/runtest\/ strings in them -- skip the rest.
/\/runtest\// {
    split ($0,strs," ")
    for (str in strs) {
        if ($str ~ /\${LTPROOT}\/runtest\/([^[:space:]]+)/) {
            sub (/\${LTPROOT}\/runtest\//, "", $str)
            print $str
        }
    }
}
