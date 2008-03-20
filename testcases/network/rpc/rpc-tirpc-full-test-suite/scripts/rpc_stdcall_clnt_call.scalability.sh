#!/bin/sh

# This test is a part of RPC & TI-RPC Test Suite created by Cyril LACABANNE
# (c) 2007 BULL S.A.S.
# Please refer to RPC & TI-RPC Test Suite documentation.
# More details at http://nfsv4.bullopensource.org/doc/rpc_testsuite.php

# TEST : RPC stdcall clnt_call scalability
# creation : 2007-07-02 revision 2007-

# **********************
# *** INITIALISATION ***
# **********************
# Parameters such as tests information, threads number...
# test information
TESTNAME="RPC_std-call_clnt_call.scalability"
TESTVERS="1.0"
# test binaries, used to call 
TESTCLIENTPATH="rpc_suite/rpc/rpc_stdcall_clnt_call"
TESTCLIENTBIN="5-scalability.bin"
TESTCLIENT=$CLIENTTSTPACKDIR/$TESTCLIENTPATH/$TESTCLIENTBIN
# table to save all tests result
result=
# tmp file declaration to store test returned result
TMPRESULTFILE=/tmp/rpcts.tmp
# table to save all instances performance
tblresult=

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
	
	for ((a=0; a < NBINSTS-1 ; a++))
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
	echo ${tblresult[*]}>>$LOCLOGDIR/$TESTLOGFILE
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
NBINSTS=1
for ((a=0; a < SCALINSTNUMBER ; a++))
do
	#echo "STEP : max insts = "$NBINSTS
	
	TIC=`echo $(date +%S)"."$(date +%N)`
	for ((i=0; i < NBINSTS ; i++))
	do
		$REMOTESHELL $CLIENTUSER@$CLIENTIP "$TESTCLIENT $SERVERIP $PROGNUMBASE $NBPERFTESTITER" >>$TMPRESULTFILE
	done
	TOC=`echo $(date +%S)"."$(date +%N)`
	
	#echo $TIC
	#echo $TOC
	CTIME=`echo $(echo "$TOC-$TIC" | bc)`
	
	if [ `expr $TOC \< $TIC` -eq 1 ]
	then
		CTIME=`echo $(echo 60 + $CTIME | bc)`
	fi
	#echo $CTIME
	
	tblresult=( "${tblresult[@]}" "("$NBINSTS"; "$CTIME") " )
	
	NBINSTS=`expr $NBINSTS \* 2`
done

NBINSTS=`expr $NBINSTS \/ 2`
#echo ${tblresult[*]}

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
