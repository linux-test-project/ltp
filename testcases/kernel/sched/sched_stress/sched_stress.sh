#!/bin/sh
#
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
#
#  FILE   : sched_stress
#
#  PURPOSE : Exports required environment variables and runs sched_driver
#

# The command below will only work on x86 setups, b/c other archs keep
# their bootfiles other locations.
export KERNEL=./sched_datafile
touch $KERNEL
echo 0.000000 > sch.measure
export RAWDEV=`df / | grep dev | awk {'print $1'}`
sched_driver -s 0.9 -t 0.02 -p 2 > /tmp/tmp$$
tail -n 5 /tmp/tmp$$
rm -rf /tmp/tmp$$ ./sched_datafile sch.measure
