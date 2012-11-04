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

set P11_SO_PWD $env(P11_SO_PWD)
set P11_USER_PWD $env(P11_USER_PWD)
set timeout 30

# Initiate the tpmtoken_init command
#   it should not have been initialized at this point
#   if it has, it is an error
spawn tpmtoken_init
expect {
	-re "Clear the TPM token data?" {
		set reinit 1
		send "n\n"
	}
	-re "A new TPM security officer password is needed." {
		set reinit 0
	}
}

if {$reinit == 0} {
	expect -re "Enter new password: "
	send "$P11_SO_PWD\n"
	expect -re "Confirm password: "
	send "$P11_SO_PWD\n"
	expect -re "A new TPM user password is needed."
	expect -re "Enter new password: "
	send "$P11_USER_PWD\n"
	expect -re "Confirm password: "
	send "$P11_USER_PWD\n"
}

expect timeout

set rc_list [wait -i $spawn_id]
set rc [lindex $rc_list {3}]

if {$reinit == 1} { set rc 1 }

exit $rc
