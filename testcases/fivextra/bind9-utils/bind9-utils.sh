#! /bin/bash
##									      ##
## (C) Copyright IBM Corp. 2004						      ##
##									      ##
## This program is free software;  you can redistribute it or modify          ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or	      ##
## (at your option) any later version.					      ##
##									      ##
## This program is distributed in the hope that it will be useful, but	      ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.							      ##
##									      ##
## You should have received a copy of the GNU General Public License	      ##
## along with this program;  if not, write to the Free Software		      ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##									      ##
################################################################################
#
# File :	bind9-utils.sh
#
# Description:  base testcase used to make sure package bind9-utils correct
#
# Author:	CSDL
#

# source the utility functions
cd `dirname $0`
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

################################################################################
# global variables
################################################################################

REQUIRED="expect ping"
INSTALLED="dig host nslookup nsupdate"

DOMAINNAMES=""      # set by tc_setup
SERVERADDR=""       # set by tc_setup
DOMAINHOSTS=""      # set by test01(dig)
DIGTYPES="A AXFR"

DIG=`which dig`
EXPECT=`which expect`
HOST=`which host`
NSLOOPUP=`which nslookup`
NSUPDATE=`which nsupdate`

#
#	tc_local_setup		tc_setup specific to this set of testcases
#
function tc_local_setup()
{
	# check installation and utilities
	tc_executes $REQUIRED
	tc_fail_if_bad $? "requried tools not found" || exit 255
	tc_executes $INSTALLED
	tc_fail_if_bad $? "bind9-utils not properly installed" || exit 255
	tc_exist_or_break /etc/resolv.conf
	tc_break_if_bad $? "couldn't read network configureation" || exit 255

	local SERVERADDRS
	# read domain names and dns server from configuration /etc/resolv.conf
	while read RESOLV
	do
		[ "$RESOLV" == "" ] && continue
		set $RESOLV
		[ "$1" == "search"     ] && { shift ; DOMAINNAMES="$DOMAINNAMES $@" ; }
		[ "$1" == "nameserver" ] && { shift ; SERVERADDRS="$SERVERADDRS $1" ; }
	done </etc/resolv.conf

	[ "$DOMAINNAMES" != ""  ]
	tc_fail_if_bad $? "no domain name found in /etc/resolv.conf" || exit 255
	[ "$SERVERADDRS" != ""  ]
	tc_fail_if_bad $? "no name servers found in /etc/resolv.conf" || exit 255

	for SERVERADDR in $SERVERADDRS
	do
		ping -c 1 $SERVERADDR > /dev/null && break
	done
	ping -c 1 $SERVERADDR > /dev/null
	tc_pass_or_fail $? "can not reach dns server $SERVERADDRS" || exit 255
}

################################################################################
# the testcase functions
################################################################################

#
#	test01	test dig function
#
function test01()
{
	DOMAINHOSTS=""
	for DOMAINNAME in $DOMAINNAMES
	do
	for DIGTYPE in $DIGTYPES
	do
		let TST_TOTAL+=1
		tc_register "dig $DOMAINNAME $DIGTYPE"
		$DIG @$SERVERADDR $DOMAINNAME $DIGTYPE >$stdout 2>$stderr
		tc_pass_or_fail $? "dig domain $DOMAINNAME failed" || continue

		local -i hostnum=0
		local -i records=0
		while read -a DIGINFO
		do
			[ "${DIGINFO[2]}" == "IN" ] && [ "${DIGINFO[3]}" == "A"     ] && \
			{
				let hostnum+=1 ; let records+=1 ;
				[ $hostnum -lt 3 ] && DOMAINHOSTS="${DOMAINHOSTS} ${DIGINFO[0]}"
			}
			[ "${DIGINFO[2]}" == "IN" ] && [ "${DIGINFO[3]}" == "CNAME" ] && \
				let records+=1
			[ "${DIGINFO[2]}" == "IN" ] && [ "${DIGINFO[3]}" == "MX"    ] && \
				let records+=1
			[ "${DIGINFO[2]}" == "IN" ] && [ "${DIGINFO[3]}" == "NS"    ] && \
				let records+=1
			[ "${DIGINFO[2]}" == "IN" ] && [ "${DIGINFO[3]}" == "SOA"   ] && \
				let records+=1
		done <$stdout
		[ $records == 0 ] && tc_warn "no RRs find in domain $DOMAINNAME with type $DIGTYPE"
	done
	done
}

#
#	test02	test host function
#
function test02()
{
	for HOSTNAME in $DOMAINHOSTS
	do
		let TST_TOTAL+=1
		tc_register "host $HOSTNAME"
		$HOST $HOSTNAME > $stdout 2>$stderr
		tc_fail_if_bad $? "host $HOSTNAME failed" || continue

		read -a HOSTINFO < $stdout
		[ "${HOSTINFO[2]}" != "has" ] || "${HOSTINFO[3]}" != "address" ]
		tc_pass_or_fail $? "host $HOSTNAME information error"
	done
}

#
#	test03	test nslookup function
#
function test03()
{
	for HOSTNAME in $DOMAINHOSTS
	do
		let TST_TOTAL+=1
		tc_register "nslookup $HOSTNAME"
		cat > $TCTMP/nslookup.exp <<-EOF
			#! $EXPECT
			set timeout 15
			spawn nslookup -sil
			expect {
				-re ".*>" {}
				timeout { send "exit \r"; exit 255 }
			}
			
			send "server $SERVERADDR \r"
			expect {
				-re ".*Default server:.*Address:.*>" {}
				timeout { send "exit \r"; exit 1 }
			}
			
			send "$HOSTNAME \r"
			expect {
				-re ".*Name:.*Address:.*>" {}
				timeout { send "exit \r"; exit 2 }
			}
			
			send "exit \r"
			exit 0
		EOF

		$EXPECT -f $TCTMP/nslookup.exp >$stdout 2>$stderr
		tc_pass_or_fail $? "nslookup $HOSTNAME failed"
	done
}


################################################################################
# main
################################################################################

tc_setup
TST_TOTAL=0

test01
test02
test03

tc_info "nsupdate test need operation rights and configurations, skiped"
