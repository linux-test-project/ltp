/******************************************************************************
 *				sync_file_range01.c
 *	    Copyright (c) International Business Machines  Corp., 2008
 *			    Email: bnpoorni@in.ibm.com
 *****************************************************************************/

/******************************************************************************/
/*									    */
/* This program is free software;  you can redistribute it and/or modify      */
/* it under the terms of the GNU General Public License as published by       */
/* the Free Software Foundation; either version 2 of the License, or	  */
/* (at your option) any later version.					*/
/*									    */
/* This program is distributed in the hope that it will be useful,	    */
/* but WITHOUT ANY WARRANTY;  without even the implied warranty of	    */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See		  */
/* the GNU General Public License for more details.			   */
/*									    */
/* You should have received a copy of the GNU General Public License	  */
/* along with this program;  if not, write to the Free Software	       */
/* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA    */
/*									    */
/******************************************************************************/

/*****************************************************************************
 *    TEST IDENTIFIER  		: sync_file_range01			  $
 *									  $
 *    EXECUTED BY	       : anyone				    $
 *
 *    TEST TITLE		: Checks for Errors from sync_file_range()
 *
 *    TEST CASE TOTAL  		: 5
 *
 *    CPU ARCHITECTURES		: All
 *
 *    AUTHOR		    : B N Poornima
 *
 *    DATE STARTED	     : 21/07/2008
 *
 *    TEST CASES
 *    (Tests sync_file_range() for different test cases as reported in the man
 *      page)
 *
 *     INPUT SPECIFICATIONS
 *	     No input needs to be specified
 *	       sync_file_data() in-puts are specified through test_data
 *
 *     OUTPUT SPECIFICATIONS
 *	     sync_file_data() error message matches with the expected error
 *		message.
 *
 *     ENVIRONMENTAL NEEDS
 *		Kernel version 2.6.17 and above
 *	      Kernel version 2.6.22 and above in case of PPC and PPC64
 *
 *     SPECIAL PROCEDURAL REQUIREMENTS
 *	     None
 *
 *     DETAILED DESCRIPTION
 *	     This is a test case for sync_file_range() system call.
 *	     This test suite tests various error messages from the system call
 *	     If the error message received matches with the expected
 *	     test is considered passed else test fails
 *
 *	     Total 5 Test Cases :-
 *	     Various error messages from the man page
 *
 *     Setup:
 *	     Setup files on which sync_file_range is to be called
 *
 *     Test:
 *	     Loop if the proper options are given.
 *	     Execute system call
 *	       Check return code.
 *	     If error obtained matches with the expected error
 *	     PASS the test, otherwise TEST FAILS
 *
 *     Cleanup:
 *	     Cleanup the temporary folder
 *
 ******************************************************************************/
#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test.h"
#include "lapi/sync_file_range.h"
#include "check_sync_file_range.h"

#ifndef SYNC_FILE_RANGE_WAIT_BEFORE
#define SYNC_FILE_RANGE_WAIT_BEFORE 1
#define SYNC_FILE_RANGE_WRITE 2	//DUMMY VALUES
#define SYNC_FILE_RANGE_WAIT_AFTER 4
#endif

#define SYNC_FILE_RANGE_INVALID 8

char *TCID = "sync_file_range01";
char filename[255];		/* file used for testing */
char spl_file[] = "/dev/null";
int filed, sfd;			/* normal and special fds */
int bfd = -1;			/* Bad file descriptor */

struct test_data_t {
	int *fd;
	off64_t offset;
	off64_t nbytes;
	unsigned int flags;
	int error;
} test_data[] = {
	{
	&bfd, 0, 1, SYNC_FILE_RANGE_WRITE, EBADF}, {
	&sfd, 0, 1, SYNC_FILE_RANGE_WAIT_AFTER, ESPIPE}, {
	&filed, -1, 1, SYNC_FILE_RANGE_WAIT_BEFORE, EINVAL}, {
	&filed, 0, -1, SYNC_FILE_RANGE_WRITE, EINVAL}, {
	&filed, 0, 1, SYNC_FILE_RANGE_INVALID, EINVAL}
};

int TST_TOTAL = sizeof(test_data) / sizeof(test_data[0]);

/* Extern Global Functions */
/******************************************************************************/
/*									    */
/* Function:    cleanup						       */
/*									    */
/* Description: Performs all one time clean up for this test on successful    */
/*	      completion,  premature exit or  failure. Closes all temporary */
/*	      files, removes all temporary directories exits the test with  */
/*	      appropriate return code by calling tst_exit() function.       */
/*									    */
/* Input:       None.							 */
/*									    */
/* Output:      None.							 */
/*									    */
/* Return:      On failure - Exits calling tst_exit(). Non '0' return code.   */
/*	      On success - Exits calling tst_exit(). With '0' return code.  */
/*									    */
/******************************************************************************/
void cleanup(void)
{

	/* close the file we have open */
	if (close(filed) == -1) {
		tst_resm(TWARN | TERRNO, "close(%s) failed", filename);
	}

	tst_rmdir();
}

/* Local  Functions */
/******************************************************************************/
/*									    */
/* Function:    setup							 */
/*									    */
/* Description: Performs all one time setup for this test. This function is   */
/*	      typically used to capture signals, create temporary dirs      */
/*	      and temporary files that may be used in the course of this    */
/*	      test.							 */
/*									    */
/* Input:       None.							 */
/*									    */
/* Output:      None.							 */
/*									    */
/* Return:      On failure - Exits by calling cleanup().		      */
/*	      On success - returns 0.				       */
/*									    */
/******************************************************************************/
void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(filename, "tmpfile_%d", getpid());
	if ((filed = open(filename, O_RDWR | O_CREAT, 0700)) == -1) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT,0700) failed", filename);
	}

	sfd = open(spl_file, O_RDWR | O_CREAT, 0700);
}

/******************************************************************************/
/*									    */
/* Function:    main							  */
/*									    */
/* Description: Entry point to this test-case. It parses all the command line */
/*	      inputs, calls the global setup and executes the test. It logs */
/*	      the test status and results appropriately using the LTP API's */
/*	      On successful completion or premature failure, cleanup() func */
/*	      is called and test exits with an appropriate return code.     */
/*									    */
/* Input:       Describe input arguments to this test-case		    */
/*	       -l - Number of iteration				     */
/*	       -v - Prints verbose output				   */
/*	       -V - Prints the version number			       */
/*									    */
/* Exit:       On failure - Exits by calling cleanup().		       */
/*	     On success - exits with 0 exit value.			  */
/*									    */
/******************************************************************************/
int main(int ac, char **av)
{

	int test_index = 0;

	tst_parse_opts(ac, av, NULL, NULL);

	if (!check_sync_file_range())
		tst_brkm(TCONF, NULL, "sync_file_range() not supported");

	setup();

	for (test_index = 0; test_index < TST_TOTAL; test_index++) {
		TEST(sync_file_range
		     (*(test_data[test_index].fd),
		      test_data[test_index].offset,
		      test_data[test_index].nbytes,
		      test_data[test_index].flags));

		if (TEST_RETURN != -1) {
			tst_resm(TFAIL,
				 "call succeeded unexpectedly (%ld != -1)",
				 TEST_RETURN);
			continue;
		}

		if (TEST_ERRNO == test_data[test_index].error) {
			tst_resm(TPASS | TTERRNO, "got expected error");
		} else {
			tst_resm(TFAIL | TTERRNO, "got unexpected error; "
				 "expected %d", test_data[test_index].error);
		}

	}

	cleanup();
	tst_exit();
}
