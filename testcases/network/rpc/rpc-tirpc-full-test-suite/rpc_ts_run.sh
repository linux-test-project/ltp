#!/bin/sh

# Admininstration tools for RPC & TI-RPC test suite
# By C. LACABANNE - cyril.lacabanne@bull.net
# Version 0.1.0 - BETA (Test & Dev purpose)
# creation : 2007-04-16 revision : 2007-07-06

# This script is a part of RPC & TI-RPC Test Suite
# (c) 2007 BULL S.A.S.
# Please refer to RPC & TI-RPC Test Suite documentation.
# More details at http://nfsv4.bullopensource.org/doc/rpc_testsuite.php

# Usage : 
# ./rpc_ts_run.sh -l tst_list_db_1 -l tst_list_db_2 ... -l tst_list_db_n ... [-v] [-m XXX] [-n X]
# where 
# -l indicates a tests script to run
# -v for verbose mode
# -m for way to run tests cases, see TESTWAY description for more information on XXX value
#	default : one server to many client
# -n numbers of instances to launch per test, X is a number
#	default : 1

# **********************
# *** INITIALISATION ***
# **********************

# Indicates in which directory tests binaries are installed
SERVERTSTPACKDIR=/home/lerouzic/benchs/ltp-full-20080930/testcases/network/rpc/rpc-tirpc-full-test-suite/tests_pack
CLIENTTSTPACKDIR=/home/lerouzic/benchs/ltp-full-20080930/testcases/network/rpc/rpc-tirpc-full-test-suite/tests_pack
# LOCALIP variable is computer IP/name used to run tests scripts
# if LOCALIP = SERVERIP = CLIENTIP, that means all tests are in local mode
# (run on the same machine)
LOCALIP=localhost
SERVERIP=localhost
CLIENTIP=localhost
SERVERUSER=root
CLIENTUSER=root
# number of threads for thread tests
NBTHREADPROCESS=4
# number of test process for stress test
NBTESTPROCESS=100
# number of test instances for performance test
NBPERFTESTITER=10
# maximal number of test instances for scalability test
SCALINSTNUMBER=8
# for run all test scripts correctly
SCRIPTSDIR=scripts
BASELOGDIR=logs
DATE=`date +%d-%m-%y_%H-%M`
LAPTIME=`date +%H:%M-%S`
LOGDIR="$BASELOGDIR/RUN_$DATE"
LOCLOGDIR="$BASELOGDIR/RUN_$DATE"
TESTLOGFILE="rpc_ts.log"
TESTRUNDETAILSLOG=./$LOGDIR/"test_run_details.log"
VERBOSE=0
# for remote call using ssh, rsh...
REMOTESHELL=ssh
# tests scripts table initialization
TSTSCRTBL=
# timeout variables
GLOBALTIMEOUT=1
SERVERTIMEOUT=1
STRESSTIMEOUT=1
SCALTIMEOUT=6

# way to run tests :
#	alone = one server, one client, use one to many or many couple with TESTINSTANCE equal to 1
#	one server to many client = one server replying to several client instances
#	many couple of client-server = many instances of client and server
#	one to many : TESTWAY = onetomany
#	many couple : TESTWAY = manycouple
# use script arguments to define other value
# below are default values
TESTWAY="onetomany"
# indicates how much instances to launch per test
TESTINSTANCE=1

# server identification number for server test program
PROGNUMBASE=536872000
# server identification number for function test
PROGNUMNOSVC=536875000

export REMOTESHELL
export CLIENTTSTPACKDIR
export SERVERTSTPACKDIR
export LOCALIP
export CLIENTIP
export SERVERIP
export SERVERUSER
export CLIENTUSER
export LOGDIR
export TESTLOGFILE
export VERBOSE
export SCRIPTSDIR
export TESTWAY
export TESTINSTANCE
export GLOBALTIMEOUT
export DATE
export SERVERTIMEOUT
export LOCLOGDIR
export STRESSTIMEOUT
export NBTHREADPROCESS
export NBTESTPROCESS
export NBPERFTESTITER
export PROGNUMBASE
export PROGNUMNOSVC
export SCALINSTNUMBER
export SCALTIMEOUT

# *****************
# *** PROCESSUS ***
# *****************

# make local logs directory
mkdir -p $LOGDIR

# screen echo
echo "*** Starting RPC & TI-RPC Test Suite ***"
echo " - Using remote shell \""$REMOTESHELL"\" to log into"
echo "      Local  : "$LOCALIP
echo "      Server : "$SERVERUSER"@"$SERVERIP
echo "      Client : "$CLIENTUSER"@"$CLIENTIP
echo " - Number of threads : "$NBTHREADPROCESS
echo " - Number of stress processes : "$NBTESTPROCESS
echo " - Number of performance test instances : "$NBPERFTESTITER
echo " - Number of scalability test iterations : "$SCALINSTNUMBER
# log test run info. into another file
echo "*** RPC & TI-RPC Test Suite : run details ***">$TESTRUNDETAILSLOG
echo " - Run date and time : "$DATE>>$TESTRUNDETAILSLOG
echo " - Start time : "$LAPTIME>>$TESTRUNDETAILSLOG
echo "      Local  : "$LOCALIP>>$TESTRUNDETAILSLOG
echo "      Server : "$SERVERUSER"@"$SERVERIP>>$TESTRUNDETAILSLOG
echo "      Client : "$CLIENTUSER"@"$CLIENTIP>>$TESTRUNDETAILSLOG
echo " - Number of threads : "$NBTHREADPROCESS>>$TESTRUNDETAILSLOG
echo " - Number of stress processes : "$NBTESTPROCESS>>$TESTRUNDETAILSLOG
echo " - Number of performance test instances : "$NBPERFTESTITER>>$TESTRUNDETAILSLOG
echo " - Number of scalability test iterations : "$SCALINSTNUMBER>>$TESTRUNDETAILSLOG


# looking for script arguments
for arg in $* 
do
	if [ "$arg" = "-l" ]
	then
		# when -l is found, following argument is a test script
		# add this test script to the main scripts table "tstlib"
		shift
		TSTSCRTBL=( "${TSTSCRTBL[@]}" "$1" )
		shift
	fi
	
	if [ "$arg" = "-m" ]
	then
		# value following -m is the way to run tests cases
		shift
		TESTWAY=$1
		shift
	fi
	
	if [ "$arg" = "-n" ]
	then
		# value following -n is the number tests cases instances to launch
		shift
		TESTINSTANCE=$1
		shift
	fi
	
	if [ "$arg" = "-v" ]
	then
		VERBOSE=1
		echo " - Using verbose mode"
		shift
	fi
done

# more display in case of verbose mode
if [ $VERBOSE -eq 1 ]
then
	echo " - Using tests list file "$tstlib
	echo " - Server binaries tests from "$SERVERTSTPACKDIR
	echo " - Client binaries tests from "$CLIENTTSTPACKDIR
	echo " - Scripts directory ./"$SCRIPTSDIR
	echo " - Tests logs directory ./"$LOGDIR
	echo " - Way to run tests cases : "$TESTWAY
	echo " - Number of tests instances : "$TESTINSTANCE
	echo " - Test case execution timeout : "$GLOBALTIMEOUT
	echo " - Server creation timeout : "$SERVERTIMEOUT
fi
echo " - Way to run tests cases : "$TESTWAY>>$TESTRUNDETAILSLOG
echo " - Number of tests instances : "$TESTINSTANCE>>$TESTRUNDETAILSLOG
echo " - Test case execution timeout : "$GLOBALTIMEOUT" sec.">>$TESTRUNDETAILSLOG
echo " - Server creation timeout : "$SERVERTIMEOUT" sec.">>$TESTRUNDETAILSLOG
echo "">>$TESTRUNDETAILSLOG

# make remotes logs directories if need on client and server tests machines
if [ "$LOCALIP" = "$CLIENTIP" ]
then
	echo " - Local client : local ip = client ip" #debug !
else
	if [ $VERBOSE -eq 1 ]
	then
		echo " - creating remote logs directory on client"
	fi
	$REMOTESHELL $CLIENTUSER@$CLIENTIP "mkdir -p /tmp/$LOGDIR"
	echo $LOGDIR
fi

#-- run each test listed in $tstlib script file
for tstssuite in ${TSTSCRTBL[*]}
do
	if [ -f "$tstssuite" ]
	then
		echo " * running "$tstssuite>>$TESTRUNDETAILSLOG
		./$tstssuite
	else
		echo "/!\ $tstssuite is not a scripts library, skipped"
	fi
done

# ***************
# *** RESULTS ***
# ***************
echo "*** Tests finished ***"

# collect logs files from client to local machine if needed
if [ "$LOCALIP" = "$CLIENTIP" ]
then
	echo -n ""
else
	if [ $VERBOSE -eq 1 ]
	then
		echo " - collecting logs file from remote client"
	fi
	scp $CLIENTUSER@$CLIENTIP:/tmp/$LOGDIR/$TESTLOGFILE $LOGDIR/ 
fi

# cleaning up before Test Suite end
# remove client log files
if [ "$LOCALIP" = "$CLIENTIP" ]
then
	echo " - no logs dir to clean up on client" #debug !
else
	if [ $VERBOSE -eq 1 ]
	then
		echo " - removing remote logs directory on client"
	fi
	$REMOTESHELL $CLIENTUSER@$CLIENTIP "rm -f -r /tmp/$LOGDIR"
fi

# end of Test Suite, show message where to find logs and exit
echo "Check all results in logs dir : ${LTPROOT}/testcases/network/rpc/rpc-tirpc-full-test-suite/$LOGDIR"
echo "">>$TESTRUNDETAILSLOG
LAPTIME=`date +%H:%M-%S`
echo " - End time : "$LAPTIME>>$TESTRUNDETAILSLOG
echo "">>$TESTRUNDETAILSLOG
echo "See results into "$TESTLOGFILE" file">>$TESTRUNDETAILSLOG
echo "This results log file can be added to tsLogParser as a test run">>$TESTRUNDETAILSLOG
echo "">>$TESTRUNDETAILSLOG
echo "*** End of test run ***">>$TESTRUNDETAILSLOG

# clean up rpcbind and or portmap
#-- Unreg all procedure
for ((a=PROGNUMNOSVC; a < `expr $PROGNUMNOSVC + $TESTINSTANCE` ; a++))
do
	$REMOTESHELL $SERVERUSER@$SERVERIP "$SERVERTSTPACKDIR/cleaner.bin $a"
done
for ((a=PROGNUMBASE; a < `expr $PROGNUMBASE + $TESTINSTANCE` ; a++))
do
	$REMOTESHELL $SERVERUSER@$SERVERIP "$SERVERTSTPACKDIR/cleaner.bin $a"
done
