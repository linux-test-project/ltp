#!/usr/bin/expect -f
#
# File:           do_ssh.tcl
# 
# Description:    This expect script will ssh to user@localhost and use
#                 ~. (diconnect) as the password. The script fails with a 
#                 non zero value if ssh was not a success.
#
# Input:          argv0 - user name
#                 argv1 - password
#
# Output:         None.
#
# Exit:           This program exits with the follwing exit codes:
#                 0  - on success.
#                 1  - usage error, program requires username and password.
#                 2  - timeout, in case password was not send.
#                 3  - failed to spawn ssh command.
#
# Author:         Manoj Iyer manjo@mail.utexas.edu
#
# History:        April 17 2003 - Created - Manoj Iyer
#		Aug 28 203 - RCP: added StrictHostKeyChecking=no

set timeout 5 

if {$argc < 2} {
    puts "\nUsage: pam_chgpasswd.tcl \[user\] \[password\]\n"
    exit 1
}

set user [lindex $argv 0]
set password [lindex $argv 1]

if [ catch {spawn -noecho ssh -o StrictHostKeyChecking=no $user} reason ] {
    send_user "failed to spawn passwd: $reason \n"
    exit 3
}

expect {
    timeout     { exit 2 }
    "password:" { send "$password\r" }
    expect eof
}

exit 0
