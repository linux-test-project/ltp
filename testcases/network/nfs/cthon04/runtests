:
#!/bin/sh
#
#       @(#)runtests	1.5 00/07/10 Connectathon testsuite
#
# Master runtests script.  Default is to run tests in each of
# basic, general, special, and lock subdirectories.  $NFSTESTDIR is
# removed before general and special tests (if previous test done)
# so that tests.init invoked from their respective runtests script
# will not ask if the test dir should be removed (since this was
# verified in the preceeding test).
#

Program=`basename $0`

passes=1
if test $# -ge 2
then
	if test x$1 = "x-N"
	then
		passes=$2
		shift
		shift
	fi
fi

if test $# = 1
then
	TESTS=$1
elif test $# = 2
then
	TESTS=$1
	TESTARG=$2
elif test $# = 3
then
	TESTS=$1
	TESTARG=$2
	TESTPATH=$3
	NFSTESTDIR=$TESTPATH
else
	InitFile="./tests.init"
	if test -f $InitFile
	then
		echo "$Program: using test defaults in $InitFile"
		. $InitFile
	else
		echo "$Program: no test defaults file ($InitFile)"
		echo "usage: $Program [-N passes] [tests [testargs [testpath]]]"
		echo "tests: -a=all, -b=basic, -g=general, -s=special, -l=lock"
		echo "testargs: -f=functional, -t=timing"
		exit 1
	fi
fi
if test x$NFSTESTDIR = x
then
	if test x$TESTPATH = x
	then
		echo "$Program: NFSTESTDIR environment variable not set"
		echo "usage: $Program [-N passes] [tests [testargs [testpath]]]"
		echo "tests: -a=all, -b=basic, -g=general, -s=special, -l=lock"
		echo "testargs: -f=functional, -t=timing"
		exit 1
	fi
	NFSTESTDIR=$TESTPATH
fi

export PATH CFLAGS LIBS NFSTESTDIR

case $TESTS in
	-a)	dirs="basic general special lock"	;;
	-b)	dirs="basic"		;;
	-g)	dirs="general"		;;
	-s)	dirs="special"		;;
	-l)	dirs="lock"		;;
esac

if test x"$dirs" = x
then
	echo "$Program: no tests specified"
	echo "usage: $Program [tests [testargs [testpath]]]"
	echo "tests: -a=all, -b=basic, -g=general, -s=special, -l=lock"
	echo "testargs: -f=functional, -t=timing"
	exit 1

fi

if test x$TESTARG = x
then
	TESTARG=-a
fi

passnum=1
while test $passnum -le $passes
do
	if test $passes -ne 1
	then
		echo "... Pass $passnum ..."
	fi

	for dir in $dirs
	do
		echo ""
		if test -d $NFSTESTDIR
		then
			rm -rf $NFSTESTDIR
		fi
		cd $dir
		sh runtests $TESTARG
		if [ $? -ne 0 ]
		then
			echo $dir tests failed
			exit 1
		fi
		cd ..
	done
	passnum=`expr $passnum + 1`
done

echo ""
rm -rf $NFSTESTDIR

echo "All tests completed"
