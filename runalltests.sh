#!/bin/sh

# 
#  7/10/02 martinjn@us.ibm.com added instance and time command line options
#

cd `dirname $0`
export LTPROOT=${PWD}
export TMPBASE="/tmp"

usage() 
{
	cat <<-END >&2
	usage: ${0##*/} [ -t duration ] [ -x instances ] [ -l logfile ]
                [ -r ltproot ] [ -d tmpdir ]
	defaults:
	duration=$duration
	instances=$instances
	logfile=$logfile
	ltproot=$LTPROOT
	tmpdir=$TMPBASE

	example: ${0##*/} -t 2h -x3 -l /tmp/ltplog.$$ -d ${PWD}
	END
exit
}

while getopts :t:x:l:r:d: arg
do      case $arg in
                t)      # In case you want to specify the time to run from the command line 
			# (2m = two minutes, 2h = two hours, etc)
			duration="-t $OPTARG" ;;
                x)      # number of ltp's to run
			instances="-x $OPTARG";;
                l)      logfile="-l $OPTARG";;
                r)      LTPROOT=$OPTARG;;
                d)      # append $$ to TMP, as it is recursively removed at end of script.
			TMPBASE=$OPTARG;;
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

cat ${LTPROOT}/runtest/syscalls ${LTPROOT}/runtest/fs ${LTPROOT}/runtest/fsx ${LTPROOT}/runtest/dio ${LTPROOT}/runtest/mm ${LTPROOT}/runtest/commands ${LTPROOT}/runtest/ipc ${LTPROOT}/runtest/sched ${LTPROOT}/runtest/float > ${TMP}/alltests

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
