################################################################################
#! /bin/sh					                               #
#                                                                              #
# History:	Oct - 17 - 2001	Created - Manoj Iyer, IBM Austin TX.           #
#				          email: manjo@austin.ibm.com          #
#                                                                              #
# File:		setup.sh		                                       #
#                                                                              #
# Description:	This script sets up the NFS directories in the remote machine  #
#		and invokes the program make_tree with parameters.             #
#			                                                       #
# Note:		If you know how to setup NFS and want to execute this test     #
#		stand alone, please read the comments in source  make_tree.c   #
#		for more info on how to.	                               #
################################################################################

#Export the data directory on RHOST
rsh -n $RHOST "/usr/sbin/exportfs -i :$TESTDIR -o rw,no_root_squash "
[[ $? -eq 0 ]] || echo "Could not export dir from $RHOST"

#Verify export
/usr/sbin/showmount -e $RHOST | grep $TESTDIR
[[ $? -eq 0 ]] || echo "$TESTDIR not exported"

#Create $LOCALDIR for mount point
mkdir $PWD/$LOCALDIR
[[ $? -eq 0 ]] || echo "Could not create $PWD/$LOCALDIR"

#Mount $TESTDIR from RHOST.
mount $RHOST:$TESTDIR $PWD/$LOCALDIR

#Change directory to new directory
cd $PWD/$LOCALDIR

#Execute the test
$OLDPWD/make_tree

#Check if test passed, on failure exit with 1
if [ $? -eq 0 ]
then
	echo "NFS test make_tree PASSED"
	exit 0
else
	echo "NFS test make_tree FAILED"
	exit 1
fi
fi
