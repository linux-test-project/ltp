:
#!/bin/sh
#
#       @(#)runtests	1.4 2001/12/07 Connectathon Testsuite
#

# If the initialization file is present, assume we are in the distribution
# tree and that we must copy the tests to the test directory.  Otherwise,
# we are in the test directory and can just run the tests

InitFile="../tests.init"

if test -f $InitFile
then
	. $InitFile
	export PATH CC CFLAGS LIBS
	echo "GENERAL TESTS: directory $NFSTESTDIR"
	mkdir $NFSTESTDIR
	if test ! -d $NFSTESTDIR
	then
		echo "Can't make directory $NFSTESTDIR"
		exit 1
	fi
	make copy DESTDIR=$NFSTESTDIR
	if test $? -ne 0
	then
		exit 1
	fi
	cd $NFSTESTDIR
else
	NFSTESTDIR=`pwd`
	export PATH
	echo "GENERAL TESTS: directory $NFSTESTDIR"
	make
fi

exec sh runtests.wrk FROM_RUNTESTS
