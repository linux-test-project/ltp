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
krb5_var="/var/kerberos"
krb5_dir="$krb5_var/krb5kdc"

installed="$inits/kadmin $krb5_sbin/kadmind $krb5_sbin/kadmin.local \
	$inits/krb5kdc $krb5_sbin/krb5kdc"

required="cat chmod cut expect grep ping"

MYREALM="FIVTEST.ORG"
domain=""       # set by mysetup
krb_server=""   # set by mysetup
kdb_passwd="kpassword"
krb_servip=""   # set by mysetup
restart_kdc=""		# set to yes if we stop currently-running kdc server
restart_kadmin=""	# set to yes if we stop currently-running kadmin server
stop_kdc=""		    # set to yes when we start our instance
stop_kadmin=""		# set to yes when we start our instance

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
	tc_pass_or_fail $? "krb5-server not properly installed" || return
	tc_root_or_break || return
	tc_exec_or_break $required || return

	# get server information
	local hostinfo=$(ping -c 1 `hostname` | grep PING) ; set $hostinfo
	krb_server=$2
	krb_servip=$5
	domain=${krb_server#*.}

	# shut down currently running kadmin, if running
	if $inits/kadmin status &>/dev/null ; then
		tc_info "Stopping currently running kadmin"
		$inits/kadmin stop &>/dev/null
		restart_kadmin="yes"
	fi

	# shut down currently running  KDC, if running
	if $inits/krb5kdc status &>/dev/null ; then
		tc_info "Stopping currently running kdc"
		$inits/krb5kdc stop &>/dev/null
		restart_kdc="yes"
	fi

	tc_info "Saving original kerberose configuration and state"

	# save original krb5kdc directory
	[ -e $krb5_var/krb5kdc ] && mv $krb5_var/krb5kdc $TCTMP/krb5kdc
	mkdir $krb5_var/krb5kdc

	# save original /etc/krb5.conf and /etc/krb5.keytab, if exists
	[ -e /etc/krb5.conf   ] && mv /etc/krb5.conf   $TCTMP/

	# create krb5.conf for these tests
	cat > /etc/krb5.conf <<-EOF
		[logging]
		 default = FILE:/var/log/krb5libs$$.log
		 kdc = FILE:/var/log/krb5kdc$$.log
		 admin_server = FILE:/var/log/kadmind$$.log

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

		[kdc]
		 profile = $krb5_dir/kdc.conf

		[appdefaults]
		 pam = {
		   debug = false
		   ticket_lifetime = 36000
		   renew_lifetime = 36000
		   forwardable = true
		   krb4_convert = false
		 }
	EOF

	# create kdc.conf file for these tests
	cat > $krb5_dir/kdc.conf <<-EOF
		[kdcdefaults]
		 acl_file = $krb5_dir/kadm5.acl
		 dict_file = /usr/share/dict/words
		 admin_keytab = $krb5_dir/kadm5.keytab
		 v4_mode = nopreauth

		[realms]
		 $MYREALM = {
		  master_key_type = des-cbc-crc
		supported_enctypes = des3-cbc-raw:normal \
		des3-cbc-raw:norealm des3-cbc-raw:onlyrealm \
		des3-cbc-sha1:normal des3-cbc-sha1:norealm \
		des3-cbc-sha1:onlyrealm des-cbc-crc:v4 des-cbc-crc:afs3 \
		des-cbc-crc:normal des-cbc-crc:norealm \
		des-cbc-crc:onlyrealm des-cbc-md4:v4 des-cbc-md4:afs3 \
		des-cbc-md4:normal des-cbc-md4:norealm \
		des-cbc-md4:onlyrealm des-cbc-md5:v4 des-cbc-md5:afs3 \
		des-cbc-md5:normal des-cbc-md5:norealm \
		des-cbc-md5:onlyrealm des-cbc-raw:v4 des-cbc-raw:afs3 \
		des-cbc-raw:normal des-cbc-raw:norealm \
		des-cbc-raw:onlyrealm des-cbc-sha1:v4 des-cbc-sha1:afs3 \
		des-cbc-sha1:normal des-cbc-sha1:norealm \
		des-cbc-sha1:onlyrealm
		 }
	EOF

	# create kadm5.acl file for these tests
	echo "*/admin@$MYREALM     *" > $krb5_dir/kadm5.acl
	return 0
}

#
#	tc_local_cleanup	cleanup specific to this set of testcases
#
function tc_local_cleanup()
{
	tc_info "Restoring original kerberose configuration and state"

	# stop our instance of kadmin, if any
	[ "$stop_kadmin" = "yes" ] && $inits/kadmin  stop &>/dev/null
	# stop our instance of kdc, if any
	[ "$stop_kdc"    = "yes" ] && $inits/krb5kdc stop &>/dev/null

	# remove log files
	[ -e /var/log/krb5libs$$.log ] && rm /var/log/krb5libs$$.log
	[ -e /var/log/krb5kdc$$.log  ] && rm /var/log/krb5kdc$$.log
	[ -e /var/log/kadmind$$.log  ] && rm /var/log/kadmind$$.log

	# restore original /etc/krb5.conf and /etc/krb5.keytab, if saved
	[ -e $TCTMP/krb5.conf   ] && mv $TCTMP/krb5.conf   /etc/

	# restore original krb5kdc directory, if any
	if [ -e $TCTMP/krb5kdc ] ; then
		rm -rf $krb5_var/krb5kdc
		mv $TCTMP/krb5kdc $krb5_var/
	fi

	# restart kdc if it was originaly running
	[ "$restart_kdc"    = "yes" ] && $inits/krb5kdc start 2>$stderr >$stdout

	# restart kadmin if it was originaly running
	[ "$restart_kadmin" = "yes" ] && $inits/kadmin  start 2>$stderr >$stdout

}

################################################################################
# the testcase functions
################################################################################

#
#	test01	create database
#
function test01()
{
	tc_register	"create kdb"

	# create the database
	local command="$krb5_sbin/kdb5_util create -r $MYREALM -s >/stdout 2>$stderr"
	local expcmd=`which expect`
	cat > $TCTMP/expect01.scr <<-EOF
		#!$expcmd -f
		set timeout 3
		proc abort {n} { exit \$n }
		spawn $command
		expect {
			timeout { abort 1 }
			"key:" { send "$kdb_passwd\r" }
		}
		expect {
			timeout { abort 2 }
			"verify:" { send "$kdb_passwd\r" }
		}
		expect eof
	EOF
	chmod +x $TCTMP/expect01.scr
	$TCTMP/expect01.scr >$stdout 2>$stderr
	tc_pass_or_fail $? "did not create database"
}

#
#	test03	add administrator principal
#
function test03()
{
	tc_register	"add admin principal"

	# add adminitrator principal
	local command="$krb5_sbin/kadmin.local"
	local expcmd=`which expect`
	cat > $TCTMP/expect03.scr <<-EOF
		#!$expcmd -f
		set timeout 3
		proc abort {n} { exit \$n }
		spawn $command
		expect {
			timeout { abort 1 }
			"kadmin.local:" {
				send "addprinc root/admin@$MYREALM\r"
			}
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
			timeout { abort 4 }
			"kadmin.local:" { send "quit\r" }
		}
		expect eof
	EOF
	chmod +x $TCTMP/expect03.scr
	$TCTMP/expect03.scr >$stdout 2>$stderr
	tc_pass_or_fail $? "did not add admin acl"
}

#
#	test05	start kdc
#
function test05()
{
	tc_register	"start kdc"

	# start the server
	stop_kdc="yes"
	$inits/krb5kdc start 2>$stderr >$stdout
	#tc_fail_if_bad $? "bad response from $inits/krb5kdc start" || return
	tc_pass_or_fail $? "bad response from $inits/krb5kdc start" || return

}

#
#	test06	start kadmin
#
function test06()
{
	tc_register	"start kadmin"

	# start the server
	stop_kadmin="yes"
	$inits/kadmin start 2>$stderr >$stdout
	tc_fail_if_bad $? "bad response from $inits/kadmin" || return

	sleep 1		# wait for logfile to be written
	local logfile=/var/log/kadmind$$.log
	cat  $logfile | grep "starting" >/dev/null
	tc_pass_or_fail $? "did not start." "Here is $logfile:" "`cat $logfile`"
}

#
#	test07	stop kadmin
#
function test07()
{
	tc_register	"stop kadmin"

	# start the server
	$inits/kadmin stop 2>$stderr >$stdout
	tc_pass_or_fail $? "did not stop"
	stop_kadmin="no"
}

#
#	test08	stop kdc
#
function test08()
{
	tc_register	"stop kdc"

	# start the server
	$inits/krb5kdc stop 2>$stderr >$stdout
	tc_pass_or_fail $? "did not stop"
	stop_kdc="no"
}

################################################################################
# main
################################################################################

TST_TOTAL=6

tc_setup

# Tests ending in a number are used to tc_setup and shut down the server.
test01 && \
test03 && \
test05 && \
test06

[ "$?" = "0" ] && cmd="continue" || cmd="quit"
while [ "$cmd" != "quit" ]
do
	echo "input command \"quit\" to quit"
	read cmd
done


[ "$stop_kadmin" == "yes" ] && test07
[ "$stop_kdc"    == "yes" ] && test08
