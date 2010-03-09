#!/bin/sh

echo "Starting the Tests"

passes=0
fails=0

if [ ! -d testdir ]; then
	mkdir -m 777 testdir
fi

echo "Test run starting at " `date`

usage() 
{
	cat <<-END >&2
	usage: ${0##*/} [ -a test name ] 

	example: ${0##*/} -a cases/aio_tio

END
	exit
}

while getopts :a: arg; do
case $arg in
	a)
		this_test=$OPTARG;;
	\?)
		echo "************** Help Info: ********************"
		usage;;
esac
done

if [ ! -n "$this_test"  ]; then
	cat <<EOF
${0##*/} : missing the test program. You must pass a test path/name for testing
EOF
	usage
	exit 1
fi

#Execute only once at present circumstances
#while [ $# -ge 1 ] ; do
	echo "Starting $this_test"
	$this_test 
	res=$?
	if [ $res -eq 0 ] ; then
		: $(( passes += 1 ))
	else
		: $(( fails += 1 ))
	fi
#done

echo "Pass: $passes Fail: $fails"
echo "Test run complete at" `date`

rm -rf testdir
