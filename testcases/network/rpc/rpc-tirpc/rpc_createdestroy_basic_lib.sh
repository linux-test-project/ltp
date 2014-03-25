#!/bin/bash

# This script is a part of RPC & TI-RPC Test Suite
# (c) 2007 BULL S.A.S.
# Please refer to RPC & TI-RPC Test Suite documentation.
# More details at http://nfsv4.bullopensource.org/doc/rpc_testsuite.php

# This scripts launch everything needed to test RPC & TI-RPC
# Never try to launch alone, run "rpc_ts_run.sh" instead
# Note : this script could be in more than one copy depending on what
#        tests series you want to run

# By C. LACABANNE - cyril.lacabanne@bull.net
# creation : 2007-05-25 revision : 2007-

# **********************
# *** INITIALISATION ***
# **********************

# simple test suite identification
TESTSUITENAME="RPC create-destroy domain"
TESTSUITEVERS="0.1"
TESTSUITEAUTH="Cyril LACABANNE"
TESTSUITEDATE="2007-"
TESTSUITECOMM=""

TESTSERVER_1_PATH="rpc_svc_1"
TESTSERVER_1_BIN="rpc_svc_1.bin"
TESTSERVER_1=$SERVERTSTPACKDIR/$TESTSERVER_1_PATH/$TESTSERVER_1_BIN

export TESTSERVER_1_PATH
export TESTSERVER_1_BIN
export TESTSERVER_1

# check if tests run locally or not
# if not, logs directory will be change to remote directory
if [ "$LOCALIP" != "$CLIENTIP" ]
then
	LOGDIR=/tmp/$LOGDIR
	if [ $VERBOSE -eq 1 ]
	then
		echo " - log dir changes to client log dir : "$LOGDIR # debug !
	fi
fi

# *****************
# *** PROCESSUS ***
# *****************

echo "*** Starting Test Suite : "$TESTSUITENAME" (v "$TESTSUITEVERS") ***"

#-- start TIRPC Server # 1 for that following tests series
if [ "$TESTWAY" = "onetomany" ]
then
	# run one server for one or more client
	$REMOTESHELL $SERVERUSER@$SERVERIP "$TESTSERVER_1 $PROGNUMBASE"&
else
	# launch as much server instances as client instances
	for ((a=0; a < TESTINSTANCE ; a++))
	do
		$REMOTESHELL $SERVERUSER@$SERVERIP "$TESTSERVER_1 `expr $PROGNUMBASE + $a`"&
	done
fi

#-- start another instance of RPC server for simple API call test
$REMOTESHELL $SERVERUSER@$SERVERIP "$TESTSERVER_1 $PROGNUMNOSVC"&

# wait for server creation and initialization
sleep $SERVERTIMEOUT

### SCRIPT LIST HERE !!! ###
./$SCRIPTSDIR/rpc_createdestroy_svc_destroy.sh
./$SCRIPTSDIR/rpc_createdestroy_svcfd_create.sh
./$SCRIPTSDIR/rpc_createdestroy_svctcp_create.sh
./$SCRIPTSDIR/rpc_createdestroy_svcudp_create.sh
./$SCRIPTSDIR/rpc_createdestroy_svcraw_create.sh
./$SCRIPTSDIR/rpc_createdestroy_svcudp_bufcreate.sh
./$SCRIPTSDIR/rpc_createdestroy_clnt_destroy.sh
./$SCRIPTSDIR/rpc_createdestroy_clnt_create.sh
./$SCRIPTSDIR/rpc_createdestroy_clntraw_create.sh
./$SCRIPTSDIR/rpc_createdestroy_clnttcp_create.sh
./$SCRIPTSDIR/rpc_createdestroy_clntudp_create.sh
./$SCRIPTSDIR/rpc_createdestroy_clntudp_bufcreate.sh

#-- Cleanup
$REMOTESHELL $SERVERUSER@$SERVERIP "killall -9 "$TESTSERVER_1_BIN

# ***************
# *** RESULTS ***
# ***************
