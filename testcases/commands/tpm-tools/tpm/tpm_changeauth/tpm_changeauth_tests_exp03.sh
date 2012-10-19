#! /usr/bin/expect -f
#
# Copyright (c) International Business Machines  Corp., 2005
#
# This program is free software;  you can redistribute it and#or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program;  if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#

set OWN_PWD $env(OWN_PWD)
set NEW_OWN_PWD $env(NEW_OWN_PWD)
set SRK_PWD $env(SRK_PWD)
set NEW_SRK_PWD $env(NEW_SRK_PWD)
set timeout 30

# Initiate the tpm_changeownerauth command to reset the owner and SRK password
#   owner and SRK password was previously changed to NEW_OWN_PWD and NEW_SRK_PWD!
spawn tpm_changeownerauth -s -o
expect -re "Enter owner password: "
send "$NEW_OWN_PWD\n"
expect -re "Enter new SRK password: "
send "$SRK_PWD\n"
expect -re "Confirm password: "
send "$SRK_PWD\n"
expect -re "Enter new owner password: "
send "$OWN_PWD\n"
expect -re "Confirm password: "
send "$OWN_PWD\n"
expect timeout

set rc_list [wait -i $spawn_id]
set rc [lindex $rc_list {3}]

exit $rc
