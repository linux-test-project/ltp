:
#!/bin/sh
#
#       @(#)runtests	1.3 2001/12/09 Connectathon Testsuite
#

# If the initialization file is present, assume we are in the distribution
# tree and that we must copy the tests to the test directory.  Otherwise,
# we are in the test directory and can just run the tests

InitFile="../tests.init"

# Save mount options because InitFile clobbers it (XXX).
mntopts=$MNTOPTIONS

if test -f $InitFile
then
	. $InitFile
	export PATH CFLAGS LIBS
	echo "SPECIAL TESTS: directory $NFSTESTDIR"
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
	echo "SPECIAL TESTS: directory $NFSTESTDIR"
fi

MNTOPTIONS=$mntopts exec sh runtests.wrk FROM_RUNTESTS
