#! /bin/bash

#
# This script is called from the directory where the test cases are.
# Not all the test cases use this.
#
# Usage: $0 profile testname1 [ testname2 ... ]
#
# This script looks for *each* line in profile matching the
# pattern "testid testname" and runs the corresponding test with the
# args defined in the line.

[ $# -lt 2 ] && { echo >&2 "$0: too few arguments (at least two)" ; exit 1 ; }
profile=$1
shift

#source $SCRIPTS_DIR/setenv.sh

profile_path=$PROFILES_DIR/$profile
# Does profile exist?
[ ! -f "$profile_path" ] && { echo >&2 "$0: Could not find profile ($profile_path)" ; exit 1 ; }

# if INSTALL_DIR != top_srcdir assume the individual tests are built and installed.
if [[ -f Makefile ]]; then
    # Compile the test cases to support stand alone runs.
    make
fi


# Run the test case
for testname in $*
do
	# Strip off comments and feed it to trivial parser.
	sed 's/#.*//' < $profile_path | while read line ; do
		set $line ""
		# Check if the line is elligible
		if [ "$1" = "$TEST_REL_DIR" -a "$2" = "$testname" ] ; then
			cmd=$2
			shift 2
			params="$*"

			if [ "$LOG_FILE" = "" ]; then
				LOG_FILE="$LOG_DIR/$LOG_FORMAT-${cmd}${params// /}.log"
			fi
			[ ! -d $LOG_DIR ] && mkdir -p $LOG_DIR

			(
				echo "--- Running testcase $cmd $params ---"
				date
				echo "Logging to $LOG_FILE"
				eval ./$cmd 2>&1 $params
				echo
				date
				echo "The $cmd test appears to have completed."
			) | tee -a $LOG_FILE
		fi
	done
done
