#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab :

################################################################################
# source the standard utility functions
################################################################################
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

inits="/etc/init.d"
krb5_bin="/usr/kerberos/bin"
krb5_sbin="/usr/kerberos/sbin"

installed="$krb5_sbin/kadmin \
	$krb5_sbin/ftpd $krb5_sbin/ktutil $krb5_sbin/telnetd \
	$krb5_bin/kinit $krb5_bin/klist $krb5_bin/kdestroy \
	$krb5_bin/ftp $krb5_bin/kpasswd $krb5_bin/ksu \
	$krb5_bin/kvno $krb5_bin/rcp $krb5_bin/rlogin \
	$krb5_bin/rsh $krb5_bin/telnet"

required="cat chmod cut diff dnsdomainname expect grep \
	hostname mv rm xinetd"

MYREALM="FIVTEST.ORG"
domain=""		# set by mysetup
kdb_passwd="kpassword"
krb_server=""   # set by mysetup, if not specified by arguments
[ "$1" != "" ] && krb_server="$1"
app_server=""   # set by mysetup
app_client=""   # set by mysetup
user1=""        # set by mysetup
user2=""        # set by mysetup
restart_xinetd="no"
stop_xinet="no"

################################################################################
# any utility functions specific to this file can go here
################################################################################

#
#	tc_local_setup		tc_setup specific to this set of testcases
#
function tc_local_setup()
{
	#check installation and environment
	tc_executes $installed
	tc_pass_or_fail $? "kerberos not properly installed" || return
	tc_root_or_break || return
	tc_exec_or_break $required || return

	#add temporary user1 and user2
	tc_add_user_or_break || return
	user1=$temp_user
	tc_add_user_or_break || return
	user2=$temp_user

	# get server and client info
	local hostname=$(hostname -s)
	domain=$(dnsdomainname)
	app_server="$hostname.$domain"
	app_client="$hostname.$domain"
	[ "$krb_server" == "" ] && krb_server="$hostname.$domain"

	# shut down currently running xinetd, if running
	if $inits/xinetd status &>/dev/null ; then
		tc_info "Stopping currently running xinetd"
		$inits/xinetd stop &>/dev/null
		restart_xinetd="yes"
	fi
	mv /etc/xinetd.conf $TCTMP/xinetd.conf

	# save original /etc/krb5.conf, if exists
	[ -e /etc/krb5.conf   ] && mv /etc/krb5.conf   $TCTMP/
	[ -e /etc/krb5.keytab ] && mv /etc/krb5.keytab $TCTMP/

	# create krb5.conf for these tests
	cat > /etc/krb5.conf <<-EOF
		[logging]
		 default = FILE:/var/log/krb5libs$$.log

		[libdefaults]
		 ticket_lifetime = 24000
		 default_realm = $MYREALM
		 dns_lookup_realm = false
		 dns_lookup_kdc = false

		[realms]
		 $MYREALM = {
		  kdc = $krb_server:88
		  admin_server = $krb_server:749
		  default_domain = $domain
		 }

		[domain_realm]
		 .$domain = $MYREALM
		 $domain = $MYREALM

		[appdefaults]
		 pam = {
		   debug = false
		   ticket_lifetime = 36000
		   renew_lifetime = 36000
		   forwardable = true
		   krb4_convert = false
		 }
	EOF

	# add user1 and user2 as remote login users
	echo "$user1@$MYREALM"        > /home/$user1/.k5login
	chown $user1.users /home/$user1/.k5login
	chmod 600 /home/$user1/.k5login
	echo "$user2@$MYREALM"        > /home/$user2/.k5login
	chown $user2.users /home/$user2/.k5login
	chmod 600 /home/$user2/.k5login
	return 0
}

#
#	tc_local_cleanup	cleanup specific to this set of testcases
#
function tc_local_cleanup()
{
	# remove log files
	[ -e /var/log/krb5libs$$.log ] && rm /var/log/krb5libs$$.log

	# stop our instance of xinetd, if any
	[ "$stop_xinet" = "yes" ] && /etc/init.d/xinetd stop >/dev/null 2>&1

	# remove temporary users: user1 and user2 and files related
	local uid=`cat /etc/passwd | grep "^$user1" | cut -f3 -d:`
	rm -rf /tmp/krb5cc_$uid
	tc_del_user_or_break $user1
	local uid=`cat /etc/passwd | grep "^$user2" | cut -f3 -d:`
	rm -rf /tmp/krb5cc_$uid
	tc_del_user_or_break $user2

	# restore original /etc/krb5.conf and /etc/krb5.keytab, if saved
	[ -e $TCTMP/krb5.conf   ] && mv $TCTMP/krb5.conf   /etc/
	[ -e $TCTMP/krb5.keytab ] && mv $TCTMP/krb5.keytab /etc/
	[ -e $TCTMP/xinetd.conf ] && mv $TCTMP/xinetd.conf /etc/

	# restart xinetd if it was originaly running
	[ "$restart_xinetd"    = "yes" ] && $inits/xinetd start >/dev/null 2>&1
}

################################################################################
# the testcase functions
################################################################################

#
#	test02	"create keytab"
#
function test02()
{
	tc_register	"create keytab"

	local command="$krb5_sbin/kadmin -p root/admin"
	local expcmd=`which expect`
	cat > $TCTMP/expect02.scr <<-EOF
		#!$expcmd -f
		set timeout 3
		proc abort {n} { exit \$n }
		spawn $command
		expect {
			timeout { abort 1 }
			"password:" { send "$kdb_passwd\r" }
		}
		expect {
			timeout { abort 1 }
			"kadmin:" {
				send "addprinc -randkey host/$app_server\r"
			}
		}
		expect {
			timeout { abort 1 }
			"kadmin:" {
				send "ktadd host/$app_server\r"
			}
		}
		expect {
			timeout { abort 1 }
			"kadmin:" {
				send "addprinc -randkey ftp/$app_server\r"
			}
		}
		expect {
			timeout { abort 1 }
			"kadmin:" {
				send "ktadd ftp/$app_server\r"
			}
		}
		expect {
			timeout { abort 2 }
			"kadmin:" { send "quit\r" }
		}
		expect eof
	EOF
	chmod +x $TCTMP/expect02.scr
	$TCTMP/expect02.scr >$stdout 2>$stderr
	tc_pass_or_fail $? "did not create keytab"
}

#
#	test04	add client principal
#
function test04()
{
	tc_register	"add client principal"

	# add client principal
	local command="$krb5_sbin/kadmin -p root/admin"
	local expcmd=`which expect`
	cat > $TCTMP/expect04.scr <<-EOF
		#!$expcmd -f
		set timeout 3
		proc abort {n} { exit \$n }
		spawn $command
		expect {
			timeout { abort 1 }
			"password:" { send "$kdb_passwd\r" }
		}
		expect {
			timeout { abort 1 }
			"kadmin:" { send "addprinc $user1\r" }
		}
		expect {
			timeout { abort 2 }
			"$MYREALM\":" { send "$kdb_passwd\r" }
		}
		expect {
			timeout { abort 3 }
			"$MYREALM\":" { send "$kdb_passwd\r" }
		}
		expect {
			timeout { abort 1 }
			"kadmin:" { send "addprinc $user2\r" }
		}
		expect {
			timeout { abort 2 }
			"$MYREALM\":" { send "$kdb_passwd\r" }
		}
		expect {
			timeout { abort 3 }
			"$MYREALM\":" { send "$kdb_passwd\r" }
		}
		expect {
			timeout { abort 7 }
			"kadmin:" { send "quit\r" }
		}
		expect eof
	EOF
	chmod +x $TCTMP/expect04.scr
	$TCTMP/expect04.scr >$stdout 2>$stderr
	tc_pass_or_fail $? "did not add client principal"
}

#
#	test0c	ksu
#
function test0c()
{
	tc_register	"root ksu"
	local myname=`echo $USER`

	# ksu to the temp user (good data is expected on stderr)
	echo "echo \$USER" | ksu $user1 >$stdout 2>/dev/null
	tc_fail_if_bad $? "unexpected response from \"ksu $user1\" command" \
		|| return

	cat $stdout | grep $user1 >/dev/null
	tc_fail_if_bad $? "did not switch to user $user1" || return

	echo $USER > $stdout
	cat $stdout | grep "$myname" >/dev/null
	tc_pass_or_fail $? "did not exit properly from ksu session"
}

#
#	test0d	ktutil
#
function test0d()
{
	tc_register	"ktutil"

	local expcmd=`which expect`
	cat > $TCTMP/expect0d.scr <<-EOF
		#!$expcmd -f
		set timeout 3
		proc abort {n} { exit \$n }
		spawn $krb5_sbin/ktutil
		expect {
			timeout { abort 1 }
			"ktutil:" { send "clear\r" }
		}
		expect {
			timeout { abort 2 }
			"ktutil:" { send "rkt /etc/krb5.keytab\r" }
		}
		expect {
			timeout { abort 3 }
			"ktutil:" { send "l\r" }
		}
		expect {
			timeout { abort 4 }
			"ktutil:" { send "exit\r" }
		}
		expect eof
	EOF
	chmod +x $TCTMP/expect0d.scr
	$TCTMP/expect0d.scr >$stdout 2>$stderr
	tc_fail_if_bad $? "bad response" || return

	# see that list command gave reasonable output
	cat $stdout | grep "$MYREALM" >/dev/null
	tc_pass_or_fail $? "expected to see \"$MYREALM\" in output"
}

#
#	test0a	kinit
#
function test0a()
{
	tc_register	"kinit"

	local expcmd=`which expect`
	cat > $TCTMP/expect0a.scr <<-EOF
		#!$expcmd -f
		set timeout 3
		proc abort {n} { exit \$n }
		spawn su - $user1
		expect {
			timeout { abort 1 }
			"$user1@$HOST" { send "kdestroy\r" }
		}
		expect {
			timeout { abort 2 }
			"$user1@$HOST" { send "kinit $user1\r" }
		}
		expect {
			timeout { abort 3 }
			"$MYREALM:" { send "$kdb_passwd\r" }
		}
		expect {
			timeout { abort 4 }
			"$user1@$HOST" { send "exit\r" }
		}
		expect eof
	EOF
	chmod +x $TCTMP/expect0a.scr
	$TCTMP/expect0a.scr >$stdout 2>$stderr
	tc_pass_or_fail $? "did not kinit"
}

#
#	test0b	klist
#
function test0b()
{
	tc_register	"klist"
	echo "klist -5" | su - $user1 2>$stderr >$stdout
	tc_fail_if_bad $? "bad response from klist" || return

	cat $stdout | grep "Expires" &>/dev/null
	tc_pass_or_fail $? "did not list tickets"
}

#
# test0e rlogin
#
function test0e()
{
	tc_register	"rlogin"

	local command="su - $user1"
	local expcmd=`which expect`
	cat > $TCTMP/expect0e.scr <<-EOF
		#!$expcmd -f
		set timeout 10
		proc abort {n} { exit \$n }
		spawn $command
		expect {
			timeout { abort 1 }
			"$user1@$HOST" { send "rlogin $app_server\r" }
		}
		expect {
			timeout { abort 1 }
			"$user1@$HOST" { send "exit\r" }
		}
		expect {
			timeout { abort 5 }
			"$user1@$HOST" { send "exit\r" }
		}
		expect eof
	EOF
	chmod +x $TCTMP/expect0e.scr
	$TCTMP/expect0e.scr >$stdout 2>$stderr
	tc_pass_or_fail $? "rlogin failed"
}

#
# test0f  client ksu
#
function test0f()
{
	tc_register     "client ksu"

	# allow password-less login from user2 to user1
	echo "$user2@$MYREALM" >> /home/$user1/.k5login
	
	local expcmd=`which expect`
	cat > $TCTMP/expect0f.scr <<-EOF
		#!$expcmd -f
		set timeout 3
		proc abort {n} { exit \$n }
		spawn su - $user2
		expect {
			timeout { abort 1 }
			"$user2@$HOST" { send "kinit\r" }
		}
		expect {
			timeout { abort 2 }
			"$MYREALM:" { send "$kdb_passwd\r" }
		}
		expect {
			timeout { abort 3 }
			"$user2@$HOST" { send "ksu $user1 -n $user2@$MYREALM\r" }
		}
		expect {
			timeout { abort 4 }
			"$user1@$HOST" { send "exit\r" }
		}
		expect {
			timeout { abort 4 }
			"$user2@$HOST" { send "exit\r" }
		}
		expect eof
	EOF
	chmod +x $TCTMP/expect0f.scr
	$TCTMP/expect0f.scr >$stdout 2>$stderr
	tc_pass_or_fail $? "did not ksu to $user1"
}

#
# test11 telnet
#
function test11()
{
	tc_register	"telnet"

	local command="su - $user1"
	local expcmd=`which expect`
	cat > $TCTMP/expect11.scr <<-EOF
		#!$expcmd -f
		set timeout 10
		proc abort {n} { exit \$n }
		spawn $command
		expect {
			timeout { abort 1 }
			"$user1@$HOST" { send "telnet -a -x $app_server\r" }
		}
		expect {
			timeout { abort 5 }
			"$user1@$HOST" { send "exit\r" }
		}
		expect {
			timeout { abort 5 }
			"$user1@$HOST" { send "exit\r" }
		}
		expect eof
	EOF
	chmod +x $TCTMP/expect11.scr
	$TCTMP/expect11.scr >$stdout 2>$stderr
	tc_pass_or_fail $? "telnet failed"
}

#
# test12 rsh
#
function test12()
{
	tc_register	"rsh"

	local command="su - $user1"
	local expcmd=`which expect`
	cat > $TCTMP/expect12.scr <<-EOF
		#!$expcmd -f
		set timeout 10
		proc abort {n} { exit \$n }
		spawn $command
		expect {
			timeout { abort 1 }
			"$user1@$HOST" { send "rsh $app_server\r" }
		}
		expect {
			timeout { abort 5 }
			"$user1@$HOST" { send "exit\r" }
		}
		expect {
			timeout { abort 5 }
			"$user1@$HOST" { send "exit\r" }
		}
		expect eof
	EOF
	chmod +x $TCTMP/expect12.scr
	$TCTMP/expect12.scr >$stdout 2>$stderr
	tc_pass_or_fail $? "rsh failed"
}

#
# test13 rcp
#
function test13()
{
	tc_register	"rcp"
	echo "$user1 temporary rcp file" > /home/$user1/rcp.src

	local command="su - $user1"
	local expcmd=`which expect`
	cat > $TCTMP/expect13.scr <<-EOF
		#!$expcmd -f
		set timeout 10
		proc abort {n} { exit \$n }
		spawn $command
		expect {
			timeout { abort 1 }
			"$user1@$HOST" { send "rcp -x rcp.src $app_server:rcp.mid\r" }
		}
		expect {
			timeout { abort 1 }
			"$user1@$HOST" { send "rcp -x $app_server:rcp.mid rcp.tgt\r" }
		}
		expect {
			timeout { abort 5 }
			"$user1@$HOST" { send "exit\r" }
		}
		expect eof
	EOF
	chmod +x $TCTMP/expect13.scr
	$TCTMP/expect13.scr >$stdout 2>$stderr && \
		diff /home/$user1/rcp.src /home/$user1/rcp.tgt
	tc_pass_or_fail $? "rcp failed"
}

#
# test14 ftp
#
function test14()
{
	tc_register	"ftp"
	echo "$user1 temporary ftp file" > /home/$user1/ftp.src

	local command="su - $user1"
	local expcmd=`which expect`
	cat > $TCTMP/expect14.scr <<-EOF
		#!$expcmd -f
		set timeout 10
		proc abort {n} { exit \$n }
		spawn $command
		expect {
			timeout { abort 1 }
			"$user1@$HOST" { send "ftp -x $app_server\r" }
		}
		expect {
			timeout { abort 1 }
			"Name"  { send "$user1\r" }
		}
		expect {
			timeout { abort 5 }
			"ftp" { send "put ftp.src ftp.mid\r" }
		}
		expect {
			timeout { abort 5 }
			"ftp" { send "get ftp.mid ftp.tgt\r" }
		}
		expect {
			timeout { abort 5 }
			"ftp" { send "bye\r" }
		}
		expect {
			timeout { abort 5 }
			"$user1@$HOST" { send "exit\r" }
		}
		expect eof
	EOF
	chmod +x $TCTMP/expect14.scr
	$TCTMP/expect14.scr >$stdout 2>$stderr && \
		diff /home/$user1/ftp.src /home/$user1/ftp.tgt
	tc_pass_or_fail $? "ftp failed"
}

#
# test15 kpasswd
#
function test15()
{
	tc_register	"kpasswd"

	local command="su - $user1"
	local expcmd=`which expect`
	cat > $TCTMP/expect15.scr <<-EOF
		#!$expcmd -f
		set timeout 10
		proc abort {n} { exit \$n }
		spawn $command
		expect {
			timeout { abort 1 }
			"$user1@$HOST"     { send "kpasswd $user1\r" }
		}
		expect {
			timeout { abort 1 }
			"$user1@$MYREALM:" { send "$kdb_passwd\r" }
		}
		expect {
			timeout { abort 5 }
			"new password:"    { send "$kdb_passwd\r" }
		}
		expect {
			timeout { abort 5 }
			"again:"           { send "$kdb_passwd\r" }
		}
		expect {
			timeout { abort 5 }
			"$user1@$HOST"     { send "exit\r" }
		}
		expect eof
	EOF
	chmod +x $TCTMP/expect15.scr
	$TCTMP/expect15.scr >$stdout 2>$stderr
	tc_pass_or_fail $? "change passwd failed"
}

#
# test16 kvno
#
function test16()
{
	tc_register	"kvno"

	local command="su - $user1"
	local expcmd=`which expect`
	cat > $TCTMP/expect16.scr <<-EOF
		#!$expcmd -f
		set timeout 10
		proc abort {n} { exit \$n }
		spawn $command
		expect {
			timeout { abort 1 }
			"$user1@$HOST" { send "kvno $user1\r" }
		}
		expect {
			timeout { abort 5 }
			"$user1@$HOST" { send "exit\r" }
		}
		expect eof
	EOF
	chmod +x $TCTMP/expect16.scr
	$TCTMP/expect16.scr >$stdout 2>$stderr
	tc_fail_if_bad $? "ticket version failed"

	# see that list command gave reasonable output
	cat $stdout | grep "$user1@$MYREALM" >/dev/null
	tc_pass_or_fail $? "expected to see \"$user1@$MYREALM\" in output"
}

#
# test00: start up application services
#
function test00()
{
	#create application server configuration
	cat > /etc/xinetd.conf <<-EOF
		defaults
		{
			instances      = 25
			log_type       = FILE $TCTMP/servicelog
			log_on_success = HOST PID
			#log_on_failure = HOST RECORD
		}
		service klogin
		{
			flags          = REUSE
			socket_type    = stream
			wait           = no
			user           = root
			server         = $krb5_sbin/klogind
			server_args    = -5
			disable        = no
		}
		service telnet
		{
			flags          = REUSE
			socket_type    = stream
			wait           = no
			user           = root
			server         = $krb5_sbin/telnetd
			disable        = no
		}
		service ftp
		{
			socket_type    = stream        
			wait           = no
			user           = root
			server         = $krb5_sbin/ftpd
			server_args    = -l -a
			disable        = no
		}
		service kshell
		{
			socket_type     = stream        
			wait            = no
			user            = root
			server          = $krb5_sbin/kshd
			server_args     = -e -5
			disable        = no
		}
	EOF

	# start application services
	/etc/init.d/xinetd start >/dev/null 2>&1 || exit 1
	stop_xinet="yes"
}

#
# testff: stop application services
#
function testff()
{
	[ "$stop_xinet" = "yes" ] && /etc/init.d/xinetd stop >/dev/null 2>&1
	stop_xinet="no"
}

################################################################################
# main
################################################################################
[ "$1" == "--help" ] && echo "$0 kdcserv" && exit 1

TST_TOTAL=13

tc_setup

#start services, exit if failed
test00

test02
test04
test0c
test0d
test0a
test0b
test0e
#test0f ksu is not working so far
test11
test12
test13
test14
test15
test16

#stop services
testff
