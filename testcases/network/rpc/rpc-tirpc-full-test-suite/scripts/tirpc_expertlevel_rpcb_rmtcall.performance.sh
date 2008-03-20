#!/bin/sh

# This test is a part of RPC & TI-RPC Test Suite created by Cyril LACABANNE
# (c) 2007 BULL S.A.S.
# Please refer to RPC & TI-RPC Test Suite documentation.
# More details at http://nfsv4.bullopensource.org/doc/rpc_testsuite.php

# TEST : TIRPC expertlevel rpcb_rmtcall performance
# creation : 2007-06-29 revision 2007-

# **********************
# *** INITIALISATION ***
# **********************
# Parameters such as tests information, threads number...
# test information
TESTNAME="TIRPC_expert-level_rpcb_rmtcall.performance"
TESTVERS="1.0"
# test binaries, used to call 
TESTCLIENTPATH="rpc_suite/tirpc/tirpc_expertlevel_rpcb_rmtcall"
TESTCLIENTBIN="7-performance.bin"
TESTCLIENT=$CLIENTTSTPACKDIR/$TESTCLIENTPATH/$TESTCLIENTBIN
# table to save all tests result
result=
# tmp file declaration to store test returned result
TMPRESULTFILE=/tmp/rpcts.tmp

# *****************
# *** PROCESSUS ***
# *****************

# erase temp. result file
echo -n "">$TMPRESULTFILE

# function to collect log result
get_test_result()
{
	# default : test failed
	r_value=1
	
	# if result table is empty last test crashes (segment fault), so return must be "failed"
	if [ ${#result[*]} -eq 0 ]
	then
		return
	fi
	
	# if all test instances return same result return the first element, note that test succeeds if value is 0
	r_value=${result[0]}
}

# function to put test result into logfile
result_to_logFile()
{
	case $r_value in
	0)r_valueTxt="PASS";;
	1)r_valueTxt="FAILED";;
	2)r_valueTxt="HUNG";;
	3)r_valueTxt="INTERRUPTED";;
	4)r_valueTxt="SKIP";;
	5)r_valueTxt="UNTESTED";;
	esac
	
	echo $TESTCLIENTPATH"/"$( echo $TESTCLIENTBIN | cut -d . -f1 )": execution: "$r_valueTxt>>$LOCLOGDIR/$TESTLOGFILE
	# print into log file tests perf results
	echo  -n "Average execution time : "${result[1]}" ms (">>$LOCLOGDIR/$TESTLOGFILE
	echo "for "${result[2]}" iterations)">>$LOCLOGDIR/$TESTLOGFILE
	echo "Minimal execution time : "${result[3]}" ms">>$LOCLOGDIR/$TESTLOGFILE
	echo "Maximal execution time : "${result[4]}" ms">>$LOCLOGDIR/$TESTLOGFILE
}

# test needs this server to run
serv=$( $REMOTESHELL $SERVERUSER@$SERVERIP "ps -e | grep $TESTSERVER_1_BIN" )
if [ -z "$serv" ]
then
	echo " - Skipped..."
	echo "/!\ Panic : no test server found"
	echo "    $TESTSERVER_1_BIN needed, but not running on server"
	echo "    Test skipped with status 4"
	r_value=4
	result_to_logFile
	echo " * $TESTNAME execution: "$r_valueTxt
	exit 4
fi

# launch client instances depeding on test...
$REMOTESHELL $CLIENTUSER@$CLIENTIP "$TESTCLIENT $SERVERIP $PROGNUMBASE $NBPERFTESTITER" >>$TMPRESULTFILE&

# wait for the end of all test
sleep $GLOBALTIMEOUT

# test if all test instances have stopped
# if it remains at least one instances, script kills instances and put status HUNG to the whole test case

IS_EX=`$REMOTESHELL $CLIENTUSER@$CLIENTIP "ps -e | grep $TESTCLIENTBIN"`

if [ "$IS_EX" ]
then
	if [ $VERBOSE -eq 1 ]
	then
		echo " - error : prog is still running -> kill"
	fi
	$REMOTESHELL $CLIENTUSER@$CLIENTIP "killall -9 $TESTCLIENTBIN"
	r_value=2
	result_to_logFile
	echo " * $TESTNAME execution: "$r_valueTxt
	exit 2
fi

# ***************
# *** RESULTS ***
# ***************

# if test program correctly run, this part aims to collect all test results and put result into log file
result=( $(cat $TMPRESULTFILE) )
get_test_result
result_to_logFile
echo " * $TESTNAME execution: "$r_valueTxt
