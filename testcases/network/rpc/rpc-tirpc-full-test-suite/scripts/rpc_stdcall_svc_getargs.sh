#!/bin/sh

# This test is a part of RPC & TI-RPC Test Suite created by Cyril LACABANNE
# (c) 2007 BULL S.A.S.
# Please refer to RPC & TI-RPC Test Suite documentation.
# More details at http://nfsv4.bullopensource.org/doc/rpc_testsuite.php

# TEST : RPC svc_getargs basic
# creation : 2007-05-31 revision 2007-06-13

# **********************
# *** INITIALISATION ***
# **********************
# Parameters such as tests information, threads number...
# test information
TESTNAME="rpc_std-call_svc_getargs.basic"
TESTVERS="1.0"
# test binaries, used to call
TESTCLIENTPATH="rpc_suite/rpc/rpc_stdcall_svc_getargs"
TESTCLIENTBIN="1-basic.bin"
TESTCLIENT=$CLIENTTSTPACKDIR/$TESTCLIENTPATH/$TESTCLIENTBIN
TESTSVCBIN="client.bin"
TESTSVC=$CLIENTTSTPACKDIR/$TESTCLIENTPATH/$TESTSVCBIN
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

	for ((a=0; a < TESTINSTANCE-1 ; a++))
	do
		if [ ${result[$a]} -ne ${result[`expr $a + 1`]} ]
		then
			return
		fi
	done

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
}

#-- This test is different from other standard test : it uses it own client/server couple
# first launch client instances depeding on test...
$REMOTESHELL $CLIENTUSER@$CLIENTIP "$TESTCLIENT $PROGNUMNOSVC" >>$TMPRESULTFILE&

sleep $GLOBALTIMEOUT

# then launch server program
$REMOTESHELL $CLIENTUSER@$CLIENTIP "$TESTSVC $CLIENTIP $PROGNUMNOSVC"&


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
	echo "from client"
	exit 2
fi

IS_EX=`$REMOTESHELL $CLIENTUSER@$CLIENTIP "ps -e | grep $TESTSVCBIN"`

if [ "$IS_EX" ]
then
	if [ $VERBOSE -eq 1 ]
	then
		echo " - error : prog is still running -> kill"
	fi
	$REMOTESHELL $CLIENTUSER@$CLIENTIP "killall -9 $TESTSVCBIN"
	r_value=2
	result_to_logFile
	echo " * $TESTNAME execution: "$r_valueTxt
	echo "from SVC"
	exit 2
fi

# ***************
# *** RESULTS ***
# ***************

# if test program correctly run, this part aims to collect all test results and put this result into log file
result=( $(cat $TMPRESULTFILE) )
get_test_result
result_to_logFile
echo " * $TESTNAME execution: "$r_valueTxt
