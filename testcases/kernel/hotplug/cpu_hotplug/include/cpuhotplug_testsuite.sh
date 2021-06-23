#!/bin/sh
############################################################
## Convenience functions for reporting, asserting, etc.   ##
############################################################

# warn(TEXT)
#
#  Issues a warning message to stderr
#
warn()
{
    echo $1 1>&2
}

# assert()
#
#  Basic assertion support.  Use it like this:
#
#   a=5
#   b=4
#   condition="$a -lt $b"     # Error message and exit from script.
#                             #  Try setting "condition" to something else,
#                             #+ and see what happens.
#
#   assert "$condition" $LINENO
#
# Note that $LINENO is a built-in
#
assert ()                 #  If condition false,
{                         #+ exit from script with error message.
  E_PARAM_ERR=98
  E_ASSERT_FAILED=99


  if [ -z "$2" ]          # Not enough parameters passed.
  then
    return $E_PARAM_ERR   # No damage done.
  fi

  lineno=$2

  if [ ! $1 ]
  then
    echo "Assertion failed:  \"$1\""
    echo "File \"$0\", line $lineno"
    exit $E_ASSERT_FAILED
  # else
  #   return
  #   and continue executing script.
  fi
}

############################################################
## Process management                                     ##
############################################################

# pid_is_valid(PID)
#
#  Checks if the given $PID is still running.  Returns a true value if
#  it is, false otherwise.
#
pid_is_valid()
{
    PID=$1
    ps --pid ${PID} --no-header | grep ${PID}
    return $?
}

# kill_pid(PID)
#
#  Forcibly kills the process ID and prevents it from
#  displaying any messages (to stdout, stderr, or otherwise)
#
kill_pid()
{
    PID=$1
    kill -9 $PID > /dev/null 2>&1
}

############################################################
## Timing                                                 ##
############################################################

# Routines in this library are set up to allow timing to be done
# by defining $TIME to a timing command.  You can define your
# own handler by defining $TIME before or after including this
# library.
TIME=${TIME:-""}

# Allows overriding the filename to use for storing time
# measurements.  Required in order to
TIME_TMP_FILE=${TIME_TMP_FILE:-"${TMP:-/tmp}/cpu_$$"}

# perform_timings()
#
#  This turns on timings for operations that support timing
#  via the $TIME variable.  It does this by setting $TIME to
#  a general purpose time command.
set_timing_on()
{
    TIME="/usr/bin/time -o $TIME_TMP_FILE -f \"%e\""
}

report_timing()
{
    MSG=${1:-"perform operation"}
    if [ ! -z "${TIME}" ]; then
        TM=`cat $TIME_TMP_FILE`
        echo "Time to ${MSG} : $TM"
    fi
}

############################################################
## Interrupt handling and cleanup                         ##
############################################################

# do_clean()
#
#  Virtual function called by do_intr().  Override this to
#  provide custom cleanup handling.
#
do_clean()
{
    return 0
}

# do_testsuite_clean()
#
#  Internal routine to do cleanup specific to other routines
#  in this testsuite.  You may override this routine if you
#  do not want this behavior.
#
do_testsuite_clean()
{
    /bin/rm -rf $TIME_TMP_FILE
}

# exit_clean(EXIT_CODE)
#
#  Replacement for exit command.  Prints the date, then calls do_clean
#  and exits with the given $EXIT_CODE, or 0 if none specified.
#
exit_clean()
{
    EXIT_CODE=${1:-0}
    date
    do_clean
    exit $EXIT_CODE
}

# do_intr()
#
#  Handler for trapped interrupts (i.e., signals 1 2 15).
#
#  This will result in a call do do_clean() when the user
#  interrupts the test, allowing you to do whatever final
#  cleanup work is needed (removing tmp files, restoring
#  resources to initial states, etc.)  This routine will
#  exit with error code 1 when done.
#
do_intr()
{
    echo "## Cleaning up... user interrupt"
    do_testsuite_clean
    do_clean
    exit 1
}

trap "do_intr" 1 2 15

