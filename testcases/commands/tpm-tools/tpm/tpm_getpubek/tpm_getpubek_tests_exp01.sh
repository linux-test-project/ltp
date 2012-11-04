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
set SRK_PWD $env(SRK_PWD)
set timeout 30

# Initiate the tpm_getpubek command
#   we should not be prompted for an owner password at this stage
#   if we are, it is an error
spawn tpm_getpubek
expect {
	-re "Public Endorsement Key" { set pwd_prompt 0 }
	-re "Enter owner password: " {
		set pwd_prompt 1
		send "$OWN_PWD\n"
	}
}
expect timeout

set rc_list [wait -i $spawn_id]
set rc [lindex $rc_list {3}]

if {$pwd_prompt == 1} { set rc 1 }

exit $rc
