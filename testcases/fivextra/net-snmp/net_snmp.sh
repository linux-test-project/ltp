#!/bin/bash

#======================================================================#
# net_snmp.sh 						               #
#								       #
# CALLS: eval_oneprogram.sh [-h][-lk] <program>		               #
#							 	       #
# SOURCES: TESTCONF.sh					               #
#								       #
# Rewritten to run within LTP harness				       #
# Robb Romans <robb@austin.ibm.com>				       #
# July 18, 2003                                                        #
#								       #
#======================================================================#

#
# source the utility functions
#
me=`which $0`
LTPBIN=${me%%/testcases/*}/testcases/bin
source $LTPBIN/tc_utils.source

tc_setup
tc_exec_or_break grep which || exit
TST_TOTAL=1 # reset in loop below
do_tests=   # set by find_tests()

#
# the source base directory for tests
#
SNMP_BASEDIR="`pwd`"
SNMP_TMPDIR="$TCTMP"
export SNMP_TMPDIR SNMP_BASEDIR

#
# Check for the configuration script.
#
[ -s "TESTCONF.sh"  ]
tc_break_if_bad $? "FATAL: No TESTCONF.sh found" || exit

[ -d "$SNMP_TMPDIR" ]
tc_break_if_bad $? "FATAL: SNMP_TMPDIR does not exist" || exit

#
# Set environment variables for test scripts
#
SNMP_VERBOSE=0                  ## 0=silent, 1=warnings, 2=more
export SNMP_VERBOSE
SNMP_SLEEP=${SNMP_SLEEP:=1}	## default seconds to sleep
export SNMP_SLEEP

#=====================================================#
# .						      #
# .Check for critical executables, add to PATH	      #
# .						      #
#=====================================================#
function check_install {

	tc_register "check snmp install"
	SBIN_PATH="/usr/sbin"
	tc_executes $SBIN_PATH/snmpd
	tc_fail_if_bad $? "FATAL: snmpd not installed?" || exit
	PATH="$SBIN_PATH:$PATH"

	INCLUDE_PATH="/usr/include"
	tc_exist_or_break $INCLUDE_PATH/net-snmp/net-snmp-config.h
	tc_fail_if_bad $? "FATAL: snmp headers not installed?" || exit

	tc_executes snmpget snmpgetnext
	tc_pass_or_fail $? "FATAL: snmp binaries not installed?" || exit
	PATH=${SNMP_BASEDIR}:$PATH
	export PATH
}

#
# Setup for the next test run.
#
rm -f core snmp_tests/core

#
# Source the testing configuration file
#
source TESTCONF.sh

#=====================#
# .		      #
# . Find Tests	      #
# .		      #
#=====================#
function find_tests {

	local num=0
	for testfile in T*; do
		case $testfile in
			# Skip backup files, and the like.
			*~)     ;;
			*.bak)  ;;
			*.orig) ;;
			*.rej)  ;;
			
			# Do the rest
			*)
				num=`expr $num + 1`
				do_tests="$do_tests $testfile"
				;;
		esac
	done
	TST_TOTAL="$num"
}

#========================#
# .			 #
# . Execute Tests	 #
# .			 #
#========================#
function run_tests {

	for testfile in $do_tests; do
		tc_register "$testfile"
		eval_onescript.sh $testfile
		tc_pass_or_fail "$?"
	done
}

#==============#
# .	       #
# .Main	       #
# .	       #
#==============#

# Switch to the testing directory
cd ./snmp_tests

check_install &&
find_tests &&
run_tests
