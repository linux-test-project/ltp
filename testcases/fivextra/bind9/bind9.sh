#!/bin/bash
# vi: set ts=8 sw=8 autoindent noexpandtab : 
################################################################################
##                                                                            ##
## (C) Copyright IBM Corp. 2004                 ##
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MEECHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
#
# File :        bind9.sh
#
# Description:  Test bind9 package
#
# Author:       
#
# History:
#       
################################################################################

# source the utility functions
cd `dirname $0`         
LTPBIN=${PWD%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source 

export TCTMP

export stderr
export stdout

export -f tc_internal_dump
export -f tc_info


NAMED=`which named`
DIG=`which dig`
RNDC=`which rndc`
NSUPDATE=`which nsupdate`
KEYGEN=`which dnssec-keygen`
SIGNER=`which dnssec-signzone`
KEYSIGNER=`which dnssec-signkey`
KEYSETTOOL=`which dnssec-makekeyset`

PERL=`which perl`

export NAMED DIG NSUPDATE KEYGEN SIGNER KEYSIGNER KEYSETTOOL PERL RNDC

function check_dep(){

   tc_register "Checking dependency."

   tc_exec_or_break $NAMED || return 1
   tc_exec_or_break $DIG   || return 1
   tc_exec_or_break $NSUPDATE || return 1
   tc_exec_or_break $KEYGEN || return 1
   tc_exec_or_break $SIGNER || return 1
   tc_exec_or_break $KEYSIGNER || return 1
   tc_exec_or_break $KEYSETTOOL || return 1
   tc_exec_or_break $PERL || return 1
   tc_exec_or_break $RNDC || return 1
 
   tc_pass_or_fail $?  "Check dependency failed."
 }

function netconfig(){

   local ns

   tc_register "Net interface $1."

   case "$1" in

    start|up)
	for ns in 1 2 3 4 5
	do
            ifconfig lo:$ns 10.53.0.$ns up netmask 255.255.255.0
	done
	;;

    stop|down)
	for ns in 5 4 3 2 1
	do
	    ifconfig lo:$ns 10.53.0.$ns down
	done

	;;

	*)
	tc_break_if_bad 1 "Wrong option for netconfig."
	return 1 
    esac

    tc_pass_or_fail $?  "Netconfig failed."
}





function tc_local_setup(){
    
    BIND9DIR=$PWD 
    
    netconfig up || return 1

 }   

function tc_local_cleanup(){

    netconfig down || return 1

 }   



function stop_server(){

local status=0
cd $1

local d
for d in ns*
do
     local pidfile="$d/named.pid"
     if [ -f $pidfile ]; then
        kill -TERM `cat $pidfile` > /dev/null 2>&1
        if [ $? != 0 ]; then
                echo "I:$d died before a SIGTERM was sent" >> $stderr
                status=`expr $status + 1`
                rm -f $pidfile
        fi
	rm -f $d/named.run
     fi
done


return $status

}




function start_server(){

local d
local status
local portup=0
local testloop=0

while [ $portup = 0 ]
do
    if $PERL $BIND9DIR/bind9test/testsock.pl -p 5300
        then
            portup=1
        else
	    sleep 1
	    testloop=`expr $testloop + 1`
	    if [ $testloop = 5 ]; then
	       echo "Could not bind to server addresses, server sockets not available." > $stderr
	       stop_server $1
	       tc_break_if_bad 1 "Testsock failed."
               return 1
            fi
        fi
done

cd $1

for d in ns[0-9]*
do
   (
      cd $d
      rm -f *.jnl *.bk *.st named.run
      if test -f namedopts
      then
          opts=`cat namedopts`
      else
          opts=""
      fi
      $NAMED $opts -c named.conf -d 99 -g >named.run 2>&1 &
      x=1
      while test ! -f named.pid
      do
         x=`expr $x + 1`
         if [ $x = 15 ]; then
            echo "Couldn't start server $d" >> $stderr
            stop_server $1
            tc_fail_if_bad 1 "Start named failed."
            return 1
         fi
         sleep 1
      done
   )
done


# Make sure all of the servers are up.

status=0

sleep 1

for d in ns[0-9]*
do
      try=0
      while true
      do
         n=`echo $d | sed 's/ns//'`
         if $DIG +tcp +noadd +nosea +nostat +noquest +nocomm +nocmd \
                 -p 5300 version.bind. chaos txt @10.53.0.$n > dig.out
         then
              break
         fi
         grep ";" dig.out >> $stderr
         try=`expr $try + 1`
         if [ $try = 30 ]; then
                stop_server $1
                rm -f dig.out
                echo "No response from $d. " >> $stderr
                tc_fail_if_bad 1 "Test started server failed."
                return 1
         fi
         sleep 1
      done
done
rm -f dig.out

}


function test_dns_forward() {

 local status

 tc_register "Test dns forward."
 
 start_server $BIND9DIR/bind9test/forward || return 1

 cd $BIND9DIR/bind9test/forward

 ./tests.sh 
 status=$?       

 ./clean.sh


 stop_server $BIND9DIR/bind9test/forward  
 tc_break_if_bad $? "Stop forward failed." || return 1 


 tc_pass_or_fail $status  "Test dns forward failed."

}
	
function test_dns_xfer() {

 local status

 tc_register "Test dns xfer."
 
 start_server $BIND9DIR/bind9test/xfer || return 1

 cd $BIND9DIR/bind9test/xfer

 ./tests.sh 
 status=$?       

 ./clean.sh


 stop_server $BIND9DIR/bind9test/xfer  
 tc_break_if_bad $? "Stop xfer failed." || return 1 


 tc_pass_or_fail $status  "Test dns xfer failed."

}
	
																																																					     
function test_dns_views () {

  local status

  tc_register "Test dns views ."

  cd $BIND9DIR/bind9test/views/

  ./setup.sh

  start_server $BIND9DIR/bind9test/views || return 1  

  cd $BIND9DIR/bind9test/views/
  
  ./tests.sh 
  status=$?  

  ./clean.sh

  stop_server $BIND9DIR/bind9test/views
  tc_break_if_bad $? "Stop views failed." || return 1

  tc_pass_or_fail $status  "Test dns views failed."

}


function test_dns_security () {

  local status

  tc_register "Test dns security ."

  cd $BIND9DIR/bind9test/dnssec/
  
  ./prereq.sh
  tc_break_if_bad $? "Prepare security failed." || return 1
  
  ./setup.sh

  start_server $BIND9DIR/bind9test/dnssec || return 1  

  cd $BIND9DIR/bind9test/dnssec/
  
  ./tests.sh 
  status=$?  

  ./clean.sh

  stop_server $BIND9DIR/bind9test/dnssec
  tc_break_if_bad $? "Stop security failed." || return 1

  tc_pass_or_fail $status  "Test dns security failed."

}



function test_dns_upforwd () {

  local status

  tc_register "Test dns upforwd ."

  cd $BIND9DIR/bind9test/upforwd/

  ./setup.sh

  start_server $BIND9DIR/bind9test/upforwd || return 1  

  cd $BIND9DIR/bind9test/upforwd/
  
  ./tests.sh 
  status=$?  

  ./clean.sh

  stop_server $BIND9DIR/bind9test/upforwd
  tc_break_if_bad $? "Stop upforwd failed." || return 1

  tc_pass_or_fail $status  "Test dns upforwd failed."

}



####################################################################################
# MAIN
####################################################################################

# Function:     main
#
# Description:  - Execute all tests, report results
#
# Exit:         - zero on success
#               - non-zero on failure
#
tc_setup
check_dep || exit 1 
test_dns_forward
test_dns_xfer
test_dns_views
test_dns_upforwd
