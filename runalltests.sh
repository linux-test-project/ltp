#!/bin/sh

# 
#  07/10/02 - Jeff Martin - martinjn@us.ibm.com: Added instance and 
#                           time command line options
#
#  12/15/02 - Manoj Iyer  - manjo@mail.utexas.edu: Added options to run 
#                           LTP under CPU, IO and MM load.
#

cd `dirname $0`
export LTPROOT=${PWD}
export TMPBASE="/tmp"

usage() 
{
	cat <<-END >&2
	usage: ${0##*/} -c [-d tmpdir] -i [ -l logfile ] -m [ -r ltproot ] 
                    [ -t duration ] [ -x instances ]
                
    -c 				Run LTP under CPU load.
    -d tmpdir       Directory where temporary files will be created.
    -i				Run LTP under heavy IO load.
    -l logfile      Log results of test in a logfile.
    -m				Run LTP under heavy memory load.
    -r ltproot      Fully qualified path where testsuite is installed.
    -t duration     Execute the testsuite for given duration in hours.
    -x instances    Run multiple instances of this testsuite.

	example: ${0##*/} -t 2h -x3 -l /tmp/ltplog.$$ -d ${PWD}
	END
exit
}


# while getopts :t:x:l:r:d:mic arg
while getopts cd:il:mr:t:x arg
do  case $arg in
	c)
			$LTPROOT/testcases/bin/genload --cpu 10 2>&1 1>/dev/null & ;;
				
	d)      # append $$ to TMP, as it is recursively 
			# removed at end of script.
			TMPBASE=$OPTARG;;
	
	i)		
			$LTPROOT/testcases/bin/genload --io 10 2>&1 1>/dev/null &
			$LTPROOT/testcases/bin/genload --hdd 10 --hdd-files \
			2>&1 1>/dev/null & ;;

	l)      logfile="-l $OPTARG";;

	m)		
			$LTPROOT/testcases/bin/genload --vm 10 --vm-chunks 10 \
			2>&1 1>/dev/null & ;;

	r)      LTPROOT=$OPTARG;;

	t)      # In case you want to specify the time 
			# to run from the command line 
			# (2m = two minutes, 2h = two hours, etc)
			duration="-t $OPTARG" ;;

	x)      # number of ltp's to run
			instances="-x $OPTARG";;

	\?)     usage;;
	esac
done

export TMP="${TMPBASE}/runalltests-$$"
mkdir -p ${TMP}

if [ -n "$instances" ]; then
  instances="$instances -O ${TMP}"
fi

cd ${TMP}
if [ $? -ne 0 ]; then
  echo "could not cd ${TMP} ... exiting"
  exit
fi

export PATH="${PATH}:${LTPROOT}/testcases/bin"

cat ${LTPROOT}/runtest/syscalls ${LTPROOT}/runtest/fs ${LTPROOT}/runtest/fsx ${LTPROOT}/runtest/dio ${LTPROOT}/runtest/mm ${LTPROOT}/runtest/commands ${LTPROOT}/runtest/ipc ${LTPROOT}/runtest/sched ${LTPROOT}/runtest/math > ${TMP}/alltests

# The fsx-linux tests use the SCRATCHDEV environment variable as a location
# that can be reformatted and run on.  Set SCRATCHDEV if you want to run 
# these tests.  As a safeguard, this is disabled.
unset SCRATCHDEV
if [ -n "$SCRATCHDEV" ]; then
  cat ${LTPROOT}/runtest/fsx >> ${TMP}/alltests
fi

# display versions of installed software
${LTPROOT}/ver_linux

${LTPROOT}/pan/pan -e -S $instances $duration -a $$ -n $$ -f ${TMP}/alltests $logfile

if [ $? -eq 0 ]; then
  echo pan reported PASS
else
  echo pan reported FAIL
fi

rm -rf ${TMP}
