# bsh
#
#   Copyright (c) International Business Machines  Corp., 2004
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
# This script, and hence the invoked test cases, need to be run from a
# directory that is NOT on the DMAPI-enabled partition (/dmtest below) being
# tested; also, the DMAPI-enabled partition must be a disk (/dev/hda7 below)
# preformatted for JFS via mkfs.fs
#
./event_sn -loglevel 4 -termlevel 4 -logname event_sn.log -mtpt /dmtest -device /dev/hda7
./event_sd -loglevel 4 -termlevel 4 -logname event_sd.log -mtpt /dmtest -device /dev/hda7
./event_an -loglevel 4 -termlevel 4 -logname event_an.log -mtpt /dmtest -device /dev/hda7
# For true results, run pmr_pre, reboot and run pmr_post to test persistent
# managed regions across reboots; however, the tests will work when run in
# succession as below
./pmr_pre -loglevel 4 -termlevel 4 -logname pmr_pre.log -mtpt /dmtest -device /dev/hda7
./pmr_post -loglevel 4 -termlevel 4 -logname pmr_post.log -mtpt /dmtest -device /dev/hda7
./event_am -loglevel 4 -termlevel 4 -logname event_am.log -mtpt /dmtest -device /dev/hda7
./invis -loglevel 4 -termlevel 4 -logname invis.log -mtpt /dmtest -device /dev/hda7
./event_us -loglevel 4 -termlevel 4 -logname event_us.log -mtpt /dmtest -device /dev/hda7
./disp -loglevel 4 -termlevel 4 -logname disp.log -mtpt /dmtest -device /dev/hda7
./objref -loglevel 4 -termlevel 4 -logname objref.log -mtpt /dmtest -device /dev/hda7
./mount -loglevel 4 -termlevel 4 -logname mount.log -mtpt /dmtest -device /dev/hda7
./token -loglevel 4 -termlevel 4 -logname token.log -mtpt /dmtest -device /dev/hda7
./right -loglevel 4 -termlevel 4 -logname right.log -mtpt /dmtest -device /dev/hda7
./mmap -loglevel 4 -termlevel 4 -logname mmap.log -mtpt /dmtest -device /dev/hda7
