#!/bin/sh

# cd $TESTHOME

. ./cases
. ./usage

trap 'echo "Termainating syslog tests due to a signal ..."; exit 1;' 1 2 6 15
STARTCASE=1
ENDCASE=
_COVERAGE_PID=1; export _COVERAGE_PID
while getopts c:s: opt
do
	case $opt in
		c) 	STARTCASE=$OPTARG	
			ENDCASE=$OPTARG;;
		s)	STARTCASE=$OPTARG;;
		\?) 	echo illegal option -- x	
			usage
			exit 1;
	esac
	shift
	shift
done

if [[ $# -ne 0 ]]; then
    usage
    exit 1
fi

if [[ -z "$ENDCASE" ]]; then
    ENDCASE=10
fi

count=$STARTCASE
while [ $count -le $ENDCASE ]
do
	echo ________________________________________________________________
	status_flag=0
	echo "			syslog test case $count"
	syslog_case$count
	if [ $status_flag -eq 0 ]
	then
		echo
		echo "syslog case $count: 	PASSED "
	else
		exit_status=$status_flag
		echo
		echo "syslog case $count: 	FAILED "
	fi
	((count = count + 1))
	echo "----------------------------------------------------------------
	"
done
exit $exit_status
