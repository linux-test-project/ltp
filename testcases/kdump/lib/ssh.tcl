set cmd [lindex $argv 0]
set pwd [lindex $argv 1]

#exp_internal 1

# 1 min timeout
#set timeout 60
set timeout -1

eval spawn $cmd

expect {
    -nocase "(yes/no)?" { send "yes\r"; exp_continue }
    -nocase "Password:" { send "$pwd\r" }
    # Handling ssh keys have already been set up.
    eof                 { exit }
    #timeout             { exit 1 }
}

# Wait for eof.
expect

exit 0
