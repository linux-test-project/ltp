################################################################################
#! /bin/sh					                               #
#                                                                              #
# History:	Oct - 17 - 2001	Created - Manoj Iyer, IBM Austin TX.           #
#				          email: manjo@austin.ibm.com          #
#                                                                              #
#		Oct - 31 - 2001 Modified.				       #
# File:		setup.sh		                                       #
#                                                                              #
# Description:	This script sets up the NFS directories in the remote machine  #
#		and invokes the program make_tree with parameters.             #
#			                                                       #
# Note:		If you know how to setup NFS and want to execute this test     #
#		stand alone, please read the comments in source  make_tree.c   #
#		for more info on how to.	                               #
################################################################################

echo -n "Enter the fully qualified name of a remote machine:"
read RHOST
echo -n "Enter the absolute path of a remote directory:"
read TESTDIR
echo -n "Enter the name of a local dir to be created:"
read LOCALDIR

echo "You have entered remote host: $RHOST, remote directory: $TESTDIR, local test directory: $LOCALDIR"


#Create the data directory on remote host.
rsh -n $RHOST "mkdir $TESTDIR" 2>&1 1>/dev/null
if [ $? -ne 0 ]
then
	echo "Could not export dir from $RHOST"
	exit 1
else
	echo "create directory $TESTDIR on remote machine $RHOST ... done"
fi


#Export the data directory on RHOST
rsh -n $RHOST "/usr/sbin/exportfs -i :$TESTDIR -o rw,no_root_squash " 2>&1 1>/dev/null
if [ $? -ne 0 ]
then
	echo "Could not export dir from $RHOST"
	exit 1
else
	echo "export data directory $TESTDIR on $RHOST ... done."
fi

#Verify export
/usr/sbin/showmount -e $RHOST | grep $TESTDIR 2>&1 1>/dev/null
if [ $? -ne 0 ]
then
	echo "$TESTDIR not exported"
	exit 1
else
	echo "export $TESTDIR on $RHOST ... done"
fi

#Create $LOCALDIR for mount point
mkdir $PWD/$LOCALDIR 2>&1 1>/dev/null
if [ $? -ne 0 ]
then
	echo "Could not create $PWD/$LOCALDIR"
	exit 1
else
	echo "create $PWD/$LOCALDIR ... done"
fi

#Mount $TESTDIR from RHOST.
/bin/mount $RHOST:$TESTDIR $PWD/$LOCALDIR 2>&1 1>/dev/null
if [ $? -ne 0 ]
then
	echo "could not mount $RHOST:$TESTDIR $PWD/$LOCALDIR"
	exit 1
else
	echo "mount $RHOST:$TESTDIR $PWD/$LOCALDIR ... done"
	echo "cd to $LOCALDIR and run ../make_tree"
fi
