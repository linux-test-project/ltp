#! /bin/sh -f
#
# TESTCONF.sh environment variable script
#==========================================#
# Modified to run within LTP		   #
# Robb Romans <robb@austin.ibm.com>	   #
# July 18, 2003				   #
#==========================================#

#====================================================================================#
# Variables:  (* = exported)							     #
#   SNMP_TESTDIR: 	  where the test scripts are kept.			     #
#  *SNMP_PERSISTENT_FILE: where to store the agent's persistent information	     #
#                         (XXX: this should be specific to just the agent)	     #
#====================================================================================#

#==================================================#
# 	     				           #
# Only allow ourselves to be eval'ed once	   #
# 						   #
#==================================================#
if [ "x$TESTCONF_SH_EVALED" != "xyes" ]; then
	TESTCONF_SH_EVALED=yes
fi

#===========================#
# .			    #
# . Global Variables	    #
# .			    #
#===========================#
if [ "x$MIBDIRS" = "x" ]; then
	MIBDIRS="/usr/share/snmp/mibs"
	export MIBDIRS
fi

#
# Set up the path to the programs we want to use.
#
if [ "x$SNMP_PATH" = "x" ]; then
	PATH=/usr/include/net-snmp/agent:$PATH
	export PATH
	SNMP_PATH=yes
	export SNMP_PATH
fi
    
if [ "x$SNMP_SAVE_TMPDIR" = "x" ]; then
    SNMP_SAVE_TMPDIR="no"
    export SNMP_SAVE_TMPDIR
fi

SNMP_TESTDIR="$SNMP_BASEDIR/snmp_tests"
SNMP_CONFIG_FILE="$SNMP_TMPDIR/snmpd.conf"
SNMPTRAPD_CONFIG_FILE="$SNMP_TMPDIR/snmptrapd.conf"
SNMP_SNMPTRAPD_LOG_FILE="$SNMP_TMPDIR/snmptrapd.log"
SNMP_SNMPTRAPD_PID_FILE="$SNMP_TMPDIR/snmptrapd.pid"
SNMP_SNMPD_PID_FILE="$SNMP_TMPDIR/snmpd.pid"
SNMP_SNMPD_LOG_FILE="$SNMP_TMPDIR/snmpd.log"
SNMP_PERSISTENT_FILE="$SNMP_TMPDIR/persistent-store.conf"
export SNMP_PERSISTENT_FILE

#
# Setup default flags and ports iff not done
#
if [ "x$SNMP_FLAGS" = "x" ]; then
    SNMP_FLAGS="-d"
fi
BASE_PORT=8765
MAX_RETRIES=3

#
# Find netstat
#
if test -x /bin/netstat ; then
    NETSTAT=/bin/netstat
elif test -x /usr/bin/netstat ; then
    NETSTAT=/usr/bin/netstat
else
    NETSTAT=""
fi

#
# Adjust the base port if necessary
# uses $RANDOM
#
if test -x $NETSTAT ; then
	if test -z "$RANDOM"; then
		RANDOM=2
	fi
	while :
	    do
	    IN_USE=`$NETSTAT -a 2>/dev/null | grep "[\.:]$BASE_PORT "`
	    if [ $? -eq 0 ]; then
		    tst_resm TINFO "Port $BASE_PORT in use: $IN_USE"
		    BASE_PORT=`expr $BASE_PORT + \( $RANDOM % 100 \)`
		    tst_resm TINFO "Using port $BASE_PORT"
	    else
		    break
	    fi
	    MAX_RETRIES=`expr $MAX_RETRIES - 1`
	    if [ $MAX_RETRIES -eq 0 ]; then
		    tst_resm TBROK "FATAL ERROR: Could not find available port."
		    exit 255
	    fi
	done
fi

if [ "x$SNMP_SNMPD_PORT" = "x" ]; then
	SNMP_SNMPD_PORT=$BASE_PORT
fi

if [ "x$SNMP_SNMPTRAPD_PORT" = "x" ]; then
	SNMP_SNMPTRAPD_PORT=`expr $BASE_PORT - 1`
fi
export SNMP_FLAGS SNMP_SNMPD_PORT SNMP_SNMPTRAPD_PORT

# Make sure the agent doesn't parse any config file but what we give it.  
# this is mainly to protect against a broken agent that doesn't
# properly handle combinations of -c and -C.  (since I've broke it before).
SNMPCONFPATH="$SNMP_TMPDIR/does-not-exist"
export SNMPCONFPATH

