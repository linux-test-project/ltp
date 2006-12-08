#! /bin/sh
#
#   Copyright (c) International Business Machines  Corp., 2001
#
#   This program is free software;  you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY;  without even the implie; warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
#   the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program;  if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#
#
#
#  FILE   : fsx.sh
#
#  PURPOSE: Runs the fsx-linux tool with a 50000 iterations setting to 
#	    attempt to uncover the "doread:read input/output" error
#	    received if the latest NFS patches for 2.4.17 from Trond
#	    are not applied. http://nfs.sf.net 
#           
#
#  SETUP: The home directory of root on the machine exported as "RHOST"
#         MUST have a ".rhosts" file with the hostname of the machine
#         where the test is executed.
#
#
#  HISTORY:
#    12/18/01 Robbie Williamson (robbiew@us.ibm.com)
#      -Written
#
#***********************************************************************

#Uncomment line below for debug output.
#trace_logic=${trace_logic:-"set -x"}

$trace_logic

#-----------------------------------------------------------------------
# Initialize local variables
#-----------------------------------------------------------------------
TC=${TC:=fsx}
TCbin=${TCbin:=`pwd`}
TCdat=${TCdat:=$TCbin}
TCsrc=${TCsrc:=$TCbin}
TCtmp=${TCtmp:=$TCbin/$TC$$}
TCdump=${TCdump:=$TCbin}

# If CLEANUP is not set; set it to "ON"
CLEANUP=${CLEANUP:="ON"}

# If EXECUTABLES is not set; set it to default executables
EXECUTABLES=${EXECUTABLES:="fsx-linux"}


#=============================================================================
# FUNCTION NAME:        setup_testcase
#
# FUNCTION DESCRIPTION: Perform the setup function for the testcase.
#
# PARAMETERS:   	None.
#
# RETURNS:      	None.
#=============================================================================

setup_testcase()
{
$trace_logic

    PID=$$
   
    VERSION=${VERSION:=2}
    RHOST=${RHOST:=`hostname`}
    ITERATIONS=${ITERATIONS:=50000}
    SOCKET_TYPE=${SOCKET_TYPE:=udp}
    TESTDIR=${TESTDIR:=/tmp/$TC$PID.testdir}
    NFS_TYPE=${NFS_TYPE:=nfs}

    echo ""
    echo "Test Options:"
    echo " VERSION: $VERSION"
    echo " RHOST: $RHOST"
    echo " ITERATIONS: $ITERATIONS"
    echo " SOCKET_TYPE: $SOCKET_TYPE"
    echo " NFS_TYPE: $NFS_TYPE"

    if [ "x$NFS_TYPE" != "xnfs4" ]; then
        OPTS=${OPTS:="-o proto=$SOCKET_TYPE,vers=$VERSION "}
    fi

    REMOTE_DIR=${RHOST}:${TESTDIR}
    LUSER=${LUSER:=root}
    mkdir -p $TCtmp || end_testcase "Could not create $TCtmp"
    chmod 777 $TCtmp

    echo "Setting up remote machine: $RHOST"
    rsh -n $RHOST "mkdir $TESTDIR"
    [ $? = 0 ] || end_testcase "Could not create remote directory"
    rsh -n $RHOST "touch $TESTDIR/testfile"
    [ $? = 0 ] || end_testcase "Could not create testfile in remote directory"

    if [ "x$NFS_TYPE" = "xnfs4" ]; then
        rsh -n $RHOST "mkdir -p /export$TESTDIR"
        [ $? = 0 ] || end_testcase "Could not create /export$TESTDIR on server"
        rsh -n $RHOST "mount --bind $TESTDIR /export$TESTDIR"
        [ $? = 0 ] || end_testcase "Could not bind $TESTDIR to export on server"
        rsh -n $RHOST "/usr/sbin/exportfs -o no_root_squash,rw,nohide,insecure,no_subtree_check *:$TESTDIR"
        [ $? = 0 ] || end_testcase "Could not export remote directory"
    else
        rsh -n $RHOST "/usr/sbin/exportfs -i -o no_root_squash,rw *:$TESTDIR"
        [ $? = 0 ] || end_testcase "Could not export remote directory"
    fi

    echo "Mounting NFS filesystem $REMOTE_DIR on $TCtmp with options '$OPTS'"
    mount -t $NFS_TYPE $OPTS $REMOTE_DIR $TCtmp || end_testcase "Cannot mount $TCtmp"
    [ $? = 0 ] || end_testcase "Could not mount $REMOTE_DIR"
}


#=============================================================================
# FUNCTION NAME:        do_test
#
# FUNCTION DESCRIPTION: Perform the test
#
# PARAMETERS:   	None.
#
# RETURNS:      	None.
#=============================================================================
do_test()
{
$trace_logic
    for executable in $EXECUTABLES
    do

        cd $TCbin
    	echo "${executable} -N $ITERATIONS $TCtmp/testfile Starting"
	./${executable} -N $ITERATIONS $TCtmp/testfile 2>&1
	retval=$?
    	echo "${executable} -N $ITERATIONS $TCtmp/testfile Finished"

	if [ "$retval" != 0 ]; then
		end_testcase "Errors have resulted from this test"
	fi

    done
}


#=============================================================================
# FUNCTION NAME:        end_testcase
#
# FUNCTION DESCRIPTION: Clean up
#
# PARAMETERS:   	None.
#
# RETURNS:      	None.
#=============================================================================
end_testcase()
{
$trace_logic
    if [ "$CLEANUP" = "ON" ]; then
	cd \
	
	echo "Cleaning up testcase"
        cd $HOME
    	echo "Unmounting $TCtmp"
	sleep 2
        umount $TCtmp || error "Cannot umount $TCtmp"
	rm -rf $TCtmp || echo "Cannot remove $TCtmp"
        rsh -n $RHOST "/usr/sbin/exportfs -u *:$TESTDIR"
 rsh -n $RHOST "rm -rf $TESTDIR"
    fi

    [ $# = 0 ] && { echo "Test Successful"; exit 0; }
    echo "Test Failed: $@"
    exit 1
}

#=============================================================================
# MAIN PROCEDURE
#=============================================================================

setup_testcase
do_test
end_testcase
