:
#!/bin/sh
#
#       @(#)runtests	1.4 99/07/16 Connectathon testsuite
#

InitFile="../tests.init"

# Save mount options because InitFile clobbers it (XXX).
mntopts="$MNTOPTIONS"

. $InitFile

if test $# -ge 1
then
	TESTARG=$1
fi

set -e
umask 0

echo "Starting LOCKING tests: test directory $NFSTESTDIR (arg: $TESTARG)"

if test ! -d $NFSTESTDIR
then
	mkdir $NFSTESTDIR
fi
if test ! -d $NFSTESTDIR
then
	echo "Can't make directory $NFSTESTDIR"
	exit 1
fi

case $TESTARG in
	-f)	TESTARGS=""	;;
	-t)	TESTARGS="-r"	;;
esac

if echo "$mntopts" | grep vers=2 > /dev/null
then
	TESTARGS="-v 2 $TESTARGS"
fi

for i in $LOCKTESTS
do
	echo ""
	case $i in
		tlock)		echo 'Testing native pre-LFS locking';;
		tlocklfs)	echo 'Testing native post-LFS locking';;
		tlock64)
			if echo "$mntopts" | grep vers=2 > /dev/null
			then
			echo "64-bit locking not supported with NFS v2"
				echo " "
				continue
			else
				echo 'Testing non-native 64 bit LFS locking'
			fi
			;;
	esac
	echo ""
	$i $TESTARGS $NFSTESTDIR
done

echo "Congratulations, you passed the locking tests!"

exit 0
