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
# This script, and hence the invoked test cases, need to be run from the
# root directory of the DMAPI-enabled partition (/dmtest below) being tested
#
./session -loglevel 4 -termlevel 4 -logname session.log
./handle -loglevel 4 -termlevel 4 -logname handle.log -mtpt /dmtest
./hole -loglevel 4 -termlevel 4 -logname hole.log
./attr -loglevel 4 -termlevel 4 -logname attr.log
./config -loglevel 4 -termlevel 4 -logname config.log
./event -loglevel 4 -termlevel 4 -logname event.log
