#!/bin/sh
################################################################################
##                                                                            ##
## Copyright (c) International Business Machines  Corp., 2008                 ##
## Copyright (c) Jinbing Guo, guojb@cn.ibm.com, 2008                          ## 
##                                                                            ##
## This program is free software;  you can redistribute it and#or modify      ##
## it under the terms of the GNU General Public License as published by       ##
## the Free Software Foundation; either version 2 of the License, or          ##
## (at your option) any later version.                                        ##
##                                                                            ##
## This program is distributed in the hope that it will be useful, but        ##
## WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY ##
## or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License   ##
## for more details.                                                          ##
##                                                                            ##
## You should have received a copy of the GNU General Public License          ##
## along with this program;  if not, write to the Free Software               ##
## Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA    ##
##                                                                            ##
################################################################################
################################################################################
# File :        run_semaphore_test_01.sh
#
# Description:  run semaphore_test_01, check the result, clean the resource.
#
# Author:       Jinbing Guo, guojb@cn.ibm.com
#
# History:      Aug. 14 2008 - Created - Jinbing Guo.
#
################################################################################
#
# Function:     setup
#
# Description:  - Check if required commands exits
#               - Export global variables
#               - Check if required config files exits
#               - Create temporary files and directories
#
# Return        - zero on success
#               - non zero on failure. return value from commands ($RC)
setup()
{
export TST_TOTAL=1  # Total number of test cases in this file.
LTPTMP=${TMP}       # Temporary directory to create files, etc.
export TCID="setup" # Test case identifier
export TST_COUNT=0  # Set up is initialized as test 0

# Initialize cleanup function to execute on program exit.
# This function will be called before the test program exits.
trap "cleanup" 0

RC=0                # Exit values of system commands used

# Set up some variables
SEM_ID=0

if [ -f $LTPROOT/testcases/bin/semaphore_test_01 ]; then
	RC=0                
else
	tst_resm TBROK, "Could not find the case semaphore_test_01."
	RC=1
fi

return $RC
}
# Function:     cleanup
#
# Description   - remove temporary files and directories.
#
# Return        - zero on success
#               - non zero on failure. return value from commands ($RC)
cleanup()
{
    if [ -n "$SEM_IPCS" ];then
        ipcrm -s $SEM_IPCS                  # remove the semaphore.
    fi
    tst_resm TINFO "CLOSE: exit."
}

# Function:     test01
#
test01()
{
TCID="semaphore_test_01"    # Identifier of this testcase.
TST_COUNT=1                 # Test case number.
RC=0                        # return code from commands.

SEM_ID=`semaphore_test_01`
RC=$?   

# Check the return value of semaphore_test_01
if [ $RC -ne 0 ]
then
tst_resm TFAIL "semaphore_test_01 failed."
return $RC
else
tst_resm TINFO "Created semaphore ID: $SEM_ID"
fi

# Get the semphore ID by "ipcs -s"
SEMS=`LANG= ipcs -s | awk '{print $2}' | grep [[:digit:]]`
for SEM_IPCS in $SEMS
do
    if [ $SEM_IPCS -eq $SEM_ID ] ;then
        tst_resm TPASS "semaphore ID comparing passed."
        return $RC
    fi
done
# Failed in comparing with the output of 'ipcs -s'
RC=1
tst_resm TFAIL "Comparing failed with the output of 'ipcs -s'."
return $RC
}
# Function:     main
#
# Description:  - Execute all tests, exit with test status.
#
# Exit:         - zero on success
#               - non-zero on failure.
#
RC=0    # Return value from setup, and test functions.
setup  || exit $RC
test01 || exit $RC

