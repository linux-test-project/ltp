#
# eval_tools.sh
#
# Output functions for script tests.  Source this from other test scripts
# to establish a standardized repertory of test functions.
#
#
# Except where noted, all functions return:
#	0	On success,	(Bourne Shell's ``true'')
#	non-0	Otherwise.
#
# Input arguments to each function are documented with each function.
#
#
# XXX  Suggestions:
#	DEBUG ON|OFF
#	dump CAPTURE output to stdout as well as to junkoutputfile.
#
#==========================================#
# Modified for LTP			   #
# Robb Romans <robb@austin.ibm.com>	   #
# July 18, 2003				   #
#==========================================#

#
# Only allow ourselves to be eval'ed once
#
if [ "x$EVAL_TOOLS_SH_EVALED" != "xyes" ]; then
	EVAL_TOOLS_SH_EVALED=yes

	if [ ! -s "TESTCONF.sh"  ] ; then
		tst_resm TBROK "`basename $0`: FATAL: No TESTCONF.sh found"
		exit 3
	fi
	source TESTCONF.sh

	if [ ! -d $TCTMP ] ; then
		tst_resm TBROK "`basename $0`: FATAL: TCTMP does not exist."
		exit 3
	fi
	#
	# Variables used in global environment of calling script.
	#
	failcount=0
	junkoutputfile="$SNMP_TMPDIR/output-`basename $0`$$"
	separator="-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-="

	#
	# HEADER: returns a single line when SNMP_HEADERONLY mode and exits.
	#
	HEADER() {
		tst_resm TINFO "testing $* "
		headerStr="testing $*"
	}

	
	OUTPUT() {	# <any_arguments>
		cat <<GRONK


$*


GRONK
	}

	CAN_USLEEP() {
		if [ "$SNMP_CAN_USLEEP" = 0 -o "$SNMP_CAN_USLEEP" = 0 ] ; then
			return $SNMP_CAN_USLEEP
		fi
		sleep .1 > /dev/null 2>&1
		if [ $? = 0 ] ; then
			SNMP_CAN_USLEEP=1
		else
			SNMP_CAN_USLEEP=0
		fi
		export SNMP_CAN_USLEEP
	}


	SUCCESS() {	# <any_arguments>
		[ "$failcount" -ne 0 ] && return
		cat <<GROINK

SUCCESS: $*

GROINK
	}


	FAILED() {	# <return_value>, <any_arguments>
		[ "$1" -eq 0 ] && return
		shift
		
		failcount=`expr $failcount + 1`
		cat <<GRONIK

FAILED: $*

GRONIK
	}

	SKIPIFNOT() {
		grep "define $1" /usr/include/net-snmp/net-snmp-config.h \
			/usr/include/net-snmp/agent/mib_module_config.h > /dev/null
		if [ $? != 0 ]; then
			REMOVETESTDATA
			tst_resm TBROK "SKIPPED: test not defined in snmp config"
			exit 357; # cause harness to fail test
		fi
	}
	
	
	VERIFY() {	# <path_to_file(s)>
		local	missingfiles=
		
		for f in $*; do
			[ -e "$f" ] && continue
			tst_resm TBROK "FAILED: Cannot find file \"$f\"."
			missingfiles=true
		done
		
		[ "$missingfiles" = true ] && exit 1000
	}


	STARTTEST() {	
		[ ! -e "$junkoutputfile" ] && {
			touch $junkoutputfile
			return
		}
		tst_resm TBROK "FAILED: Output file already exists: \"$junkoutputfile\"."
		exit 1000
	}


	STOPTEST() {
		rm -f "$junkoutputfile"
	}
	

	REMOVETESTDATA() {
		rm -rf $SNMP_TMPDIR/*
	}


	CAPTURE() {	# <command_with_arguments_to_execute>
		echo $* >> $SNMP_TMPDIR/invoked
		
		if [ $SNMP_VERBOSE -gt 0 ]; then
			cat <<KNORG

EXECUTING: $*

KNORG
			
		fi
		( $* 2>&1 ) > $junkoutputfile 2>&1
		
		if [ $SNMP_VERBOSE -gt 1 ]; then
			echo "Command Output: "
			echo "MIBDIR $MIBDIRS $MIBS"
			echo "$seperator"
			cat $junkoutputfile | sed 's/^/  /'
			echo "$seperator"
		fi
	}

	#
	# Delay to let processes settle
	#
	DELAY() {
		if [ "$SNMP_SLEEP" != "0" ] ; then
			sleep $SNMP_SLEEP
		fi
	}

	SAVE_RESULTS() {
		real_return_value=$return_value
	}

	#
	# Checks the output result against what we expect.
	# Sets return_value to 0 or 1.
	#
	EXPECTRESULT() {
		if [ "$snmp_last_test_result" = "$1" ]; then
			return_value=0
		else
			return_value=1
		fi
	}

	#
	# Returns: Count of matched lines.
	#
	CHECK() {	# <pattern_to_match>
		if [ $SNMP_VERBOSE -gt 0 ]; then
			echo -n "checking output for \"$*\"..."
		fi
		
		rval=`grep -c "$*" "$junkoutputfile" 2>/dev/null`
		
		if [ $SNMP_VERBOSE -gt 0 ]; then
			echo "$rval matches found"
		fi
		
		snmp_last_test_result=$rval
		EXPECTRESULT 1  # default
		return $rval
	}


	CHECKFILE() {
		file=$1
		if [ "x$file" = "x" ] ; then
			file=$junkoutputfile
		fi
		shift
		myoldjunkoutputfile="$junkoutputfile"
		junkoutputfile="$file"
		CHECK $*
		junkoutputfile="$myoldjunkoutputfile"
	}


	CHECKTRAPD() {
		CHECKFILE $SNMP_SNMPTRAPD_LOG_FILE $@
	}


	CHECKAGENT() {
		CHECKGAENT $SNMP_SNMPD_LOG_FILE $@
	}

	
	WAITFORAGENT() {
		WAITFOR "$@" $SNMP_SNMPD_LOG_FILE
	}
	

	WAITFORTRAPD() {
		WAITFOR "$@" $SNMP_SNMPTRAPD_LOG_FILE
	}
	

	WAITFOR() {
		sleeptime=$SNMP_SLEEP
		oldsleeptime=$SNMP_SLEEP
		if [ "$1" != "" ] ; then
			CAN_USLEEP
			if [ $SNMP_CAN_USLEEP = 1 ] ; then
				sleeptime=`expr $SNMP_SLEEP '*' 10`
			SNMP_SLEEP=.1
		else 
			SNMP_SLEEP=1
		fi
		while [ $sleeptime -gt 0 ] ; do
			if [ "$2" = "" ] ; then
				CHECK "$@"
			else
				CHECKFILE "$2" "$1"
			fi
			if [ "$snmp_last_test_result" != "" ] ; then
				if [ "$snmp_last_test_result" -gt 0 ] ; then
					SNMP_SLEEP=$oldsleeptime
					return 0;
				fi
			fi
			DELAY
			sleeptime=`expr $sleeptime - 1`
		done
		SNMP_SLEEP=$oldsleeptime
	else
		if [ $SNMP_SLEEP -ne 0 ] ; then
			sleep $SNMP_SLEEP
		fi
	fi
}


	# WAITFORORDIE "grep string" ["file"]
	WAITFORORDIE() {
		WAITFOR "$1" "$2"
		if [ "$snmp_last_test_result" != 0 ] ; then
			FINISHED
		fi
		echo "."
	}

	# CHECKFILE "grep string" ["file"]
	CHECKORDIE() {
		CHECKFILE "$2" "$1"
		if [ "$snmp_last_test_result" = 0 ] ; then
			FINISHED
		fi
		echo "."
	}


	#
	# Returns: Count of matched lines.
	#
	CHECKEXACT() {	# <pattern_to_match_exactly>
		rval=`grep -wc "$*" "$junkoutputfile" 2>/dev/null`
		snmp_last_test_result=$rval
		EXPECTRESULT 1  # default
		return $rval
	}


	CONFIGAGENT() {
		if [ "x$SNMP_CONFIG_FILE" = "x" ]; then
			tst_resm TBROK "$0: failed because var: SNMP_CONFIG_FILE wasn't set"
			exit 1;
		fi
		echo $* >> $SNMP_CONFIG_FILE
	}


	CONFIGTRAPD() {
		if [ "x$SNMPTRAPD_CONFIG_FILE" = "x" ]; then
			tst_resm TBROK "$0: failed because var: SNMPTRAPD_CONFIG_FILE wasn't set"
			exit 1;
		fi
		echo $* >> $SNMPTRAPD_CONFIG_FILE
	}



	#================================================#
	# common to STARTAGENT and STARTTRAPD		 #
	# log command to "invoked" file			 #
	# delay after command to allow for settle	 #
	#================================================#
	STARTPROG() {
		if [ $SNMP_VERBOSE -gt 1 ]; then
			echo "$CFG_FILE contains: "
			if [ -f $CFG_FILE ]; then
				cat $CFG_FILE
			else
				echo "[no config file]"
			fi
		fi
		if test -f $CFG_FILE; then
			COMMAND="$COMMAND -C -c $CFG_FILE"
		fi
		if [ $SNMP_VERBOSE -gt 0 ]; then
			tst_resm TINFO "running: $COMMAND"
		fi
		if [ "x$PORT_SPEC" != "x" ]; then
			COMMAND="$COMMAND $PORT_SPEC"
		fi
		echo $COMMAND >> $SNMP_TMPDIR/invoked
		$COMMAND > $LOG_FILE.stdout 2>&1
		if [ $? -ne 0 ] ; then
			tst_resm TFAIL "STARTPROG failed!"
			cat $LOG_FILE.stdout
			exit 357
		fi
		DELAY
	}


	STARTPROGNOSLEEP() {
		sleepxxx=$SNMP_SLEEP
		SNMP_SLEEP=0
		STARTPROG
		SNMP_SLEEP=$sleepxxx
	}


	STARTAGENT() {
		SNMPDSTARTED=1
		COMMAND="snmpd $SNMP_FLAGS -r -U -P $SNMP_SNMPD_PID_FILE -l $SNMP_SNMPD_LOG_FILE $AGENT_FLAGS"
		CFG_FILE=$SNMP_CONFIG_FILE
		LOG_FILE=$SNMP_SNMPD_LOG_FILE
		PORT_SPEC="$SNMP_SNMPD_PORT"
		
		STARTPROGNOSLEEP
		WAITFORAGENT "NET-SNMP version"
	}


	STARTTRAPD() {
		TRAPDSTARTED=1
		COMMAND="snmptrapd -d -u $SNMP_SNMPTRAPD_PID_FILE -o $SNMP_SNMPTRAPD_LOG_FILE"
		CFG_FILE=$SNMPTRAPD_CONFIG_FILE
		LOG_FILE=$SNMP_SNMPTRAPD_LOG_FILE
		PORT_SPEC="$SNMP_SNMPTRAPD_PORT"
		
		STARTPROGNOSLEEP
		WAITFORTRAPD "NET-SNMP version"
	}



	#=============================================================#
	# used by STOPAGENT and STOPTRAPD			      #
	# delay before kill to allow previous action to finish	      #
	# this is especially important for interaction between	      #
	#    master agent and sub agent.			      #
	#=============================================================#
	STOPPROG() {
		if [ -f $1 ]; then
			COMMAND="kill -TERM `cat $1`"
			echo $COMMAND >> $SNMP_TMPDIR/invoked

			DELAY
			$COMMAND > /dev/null 2>&1
		fi
	}

	STOPPROGNOSLEEP() {
		sleepxxx=$SNMP_SLEEP
		SNMP_SLEEP=0
		STOPPROG "$1"
		SNMP_SLEEP=$sleepxxx
	}


	STOPAGENT() {
		SAVE_RESULTS
		STOPPROGNOSLEEP $SNMP_SNMPD_PID_FILE
		WAITFORAGENT "shutting down"
		if [ $SNMP_VERBOSE -gt 1 ]; then
			echo "Agent Output:"
			echo "$seperator [stdout]"
			cat $SNMP_SNMPD_LOG_FILE.stdout
			echo "$seperator [logfile]"
			cat $SNMP_SNMPD_LOG_FILE
			echo "$seperator"
		fi
	}


	STOPTRAPD() {
		SAVE_RESULTS
		STOPPROGNOSLEEP $SNMP_SNMPTRAPD_PID_FILE
		WAITFORTRAPD "Stopped"
		if [ $SNMP_VERBOSE -gt 1 ]; then
			echo "snmptrapd Output:"
			echo "$seperator [stdout]"
			cat $SNMP_SNMPTRAPD_LOG_FILE.stdout
			echo "$seperator [logfile]"
			cat $SNMP_SNMPTRAPD_LOG_FILE
			echo "$seperator"
		fi
	}


	FINISHED() {
		if [ "$SNMPDSTARTED" = "1" ] ; then
			STOPAGENT
		fi
		if [ "$TRAPDSTARTED" = "1" ] ; then
			STOPTRAPD
		fi
		for pfile in $SNMP_TMPDIR/*pid* ; do
			pid=`cat $pfile`
			ps -e | egrep "^[ ]*$pid" > /dev/null 2>&1
			if [ $? = 0 ] ; then
				SNMP_SAVE_TMPDIR=yes
				COMMAND="kill -9 $pid"
				echo $COMMAND "($pfile)" >> $SNMP_TMPDIR/invoked
				$COMMAND > /dev/null 2>&1
				return_value=1
			fi
		done
		if [ "x$real_return_value" != "x0" ]; then
			if [ -s core ] ; then
				# XX hope that only one prog cores !
				cp core $SNMP_TMPDIR/core.$$
				rm -f core
			fi
			# "FAIL"
			echo "$headerStr...FAIL" >> $SNMP_TMPDIR/invoked
			exit $real_return_value
		fi
		
		#echo "ok"
		echo "$headerStr...ok" >> $SNMP_TMPDIR/invoked
		
		if [ "x$SNMP_SAVE_TMPDIR" != "xyes" ]; then
			REMOVETESTDATA
		fi
		exit $real_return_value
	}


	VERBOSE_OUT() {
		if [ $SNMP_VERBOSE > $1 ]; then
			shift
			echo "$*"
		fi
	}
	
fi # Only allow ourselves to be eval'ed once
