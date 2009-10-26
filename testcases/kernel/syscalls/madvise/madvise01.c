/***************************************************************************
 *           madvise01 .c
 *
 *  Fri May 13 15:00:59 2004
 *  Copyright (c) International Business Machines  Corp., 2004
 *  Email : sumit@in.ibm.com
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/**********************************************************
 *
 *    OS Test - International Business Machines Corp. 2004.
 *
 *    TEST IDENTIFIER	: madvise01
 *
 *    EXECUTED BY		: anyone
 *
 *    TEST TITLE		: Basic test for madvise(2)
 *
 *    TEST CASE TOTAL	: 5
 *
 *    CPU TYPES			: Intel(R) XEON(TM)
 *
 *    AUTHOR			: Sumit Sharma
 *
 *    CO-PILOT			:
 *
 *    DATE STARTED		: 13/05/2004
 *
 *    TEST CASES
 *
 * 	1.) madvise(2) advices...(See Description)
 *
 *	INPUT SPECIFICATIONS
 * 		The standard options for system call tests are accepted.
 *		(See the parse_opts(3) man page).
 *
 *	OUTPUT SPECIFICATIONS
 *		Output describing whether test cases passed or failed.
 *$
 *	ENVIRONMENTAL NEEDS
 *		None
 *
 *	SPECIAL PROCEDURAL REQUIREMENTS
 * 		None
 *
 *	DETAILED DESCRIPTION
 *		This is a test case for madvise(2) system call.
 *		It tests madvise(2) with combinations of advice values.
 *		No error should be returned.
 *
 *		Total 5 Test Cases :-
 *		(1) Test Case for MADV_NORMAL
 *		(2) Test Case for MADV_RANDOM
 *		(3) Test Case for MADV_SEQUENTIAL
 *		(4) Test Case for MADV_WILLNEED
 *		(5) Test Case for MADV_DONTNEED
 *
 *	Setup:
 *		Setup signal handling.
 *		Pause for SIGUSR1 if option specified.
 *
 *	Test:
 *		Loop if the proper options are given.
 *		Execute system call
 *		Check return code, if system call failed (return=-1)
 *		Log the errno and Issue a FAIL message.
 *		Otherwise, Issue a PASS message.
 *
 *	Cleanup:
 *		Print errno log and/or timing stats if options given
 *
 *
 *#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#*#**/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#include "test.h"
#include "usctest.h"

/* Uncomment the following line in DEBUG mode */
//#define MM_DEBUG 1

void setup(void);
void cleanup(void);
void check_and_print(char *advice);

char *TCID = "madvise01";	/* Test program modifier */
int TST_TOTAL = 5;		/* Total no of test cases */
extern int Tst_count;		/* Test case counter for tst_* routines */

int i = 0;			/* Loop Counters */

int main(int argc, char *argv[])
{
	int lc, fd;
	char *file = NULL;
	struct stat stat;

	char *msg = NULL;
	char filename[64];
	char *progname = NULL;
	char *str_for_file = "abcdefghijklmnopqrstuvwxyz12345\n";	/* 32-byte string */

	if ((msg =
	     parse_opts(argc, argv, (option_t *) NULL, NULL)) != (char *)NULL) {
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);
	}

	/**************************************************
	 *	Perform global setup for test
	 **************************************************/
	setup();

	/* Creating file in tmp directory for testing */
	progname = *argv;
	sprintf(filename, "%s-out.%d", progname, getpid());

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		/* Reset Tst_count in case we are looping */
		Tst_count = 0;

		/* Create a temporary file for testing */
		if ((fd = open(filename, O_RDWR | O_CREAT, 0664)) < 0) {
			tst_brkm(TBROK, cleanup,
				 "Could not open file \"%s\" with O_RDWR",
				 filename);
		}
#ifdef MM_DEBUG
		tst_resm(TINFO, "filename = %s opened successfully", filename);
#endif

		/* Writing 40 KB of random data into this file
		   [32 * 1280 = 40960] */
		for (i = 0; i < 1280; i++) {
			if (write(fd, str_for_file, strlen(str_for_file)) < 0) {
				tst_brkm(TBROK, cleanup,
					 "Could not write data to file \"%s\"",
					 filename);
			}
		}

		/* Get file status for its size */
		if (fstat(fd, &stat) < 0) {
			tst_brkm(TBROK, cleanup,
				 "Could not stat file \"%s\"", filename);
		}

		/* Map the input file into memory */
		if ((file =
		     (char *)mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd,
				  0)) == (char *)-1) {
			tst_brkm(TBROK, cleanup, "Could not mmap file \"%s\"",
				 filename);
		}

		/*(1) Test case for MADV_NORMAL */
		TEST(madvise(file, stat.st_size, MADV_NORMAL));
		check_and_print("MADV_NORMAL");

		/*(2) Test case for MADV_RANDOM */
		TEST(madvise(file, stat.st_size, MADV_RANDOM));
		check_and_print("MADV_RANDOM");

		/*(3) Test case for MADV_SEQUENTIAL */
		TEST(madvise(file, stat.st_size, MADV_SEQUENTIAL));
		check_and_print("MADV_SEQUENTIAL");

		/*(4) Test case for MADV_WILLNEED */
		TEST(madvise(file, stat.st_size, MADV_WILLNEED));
		check_and_print("MADV_WILLNEED");

		/*(5) Test case for MADV_DONTNEED */
		TEST(madvise(file, stat.st_size, MADV_DONTNEED));
		check_and_print("MADV_DONTNEED");

		/* Finally Unmapping the whole file */
		if (munmap(file, stat.st_size) < 0) {
			tst_brkm(TBROK, cleanup, "Could not unmap memory");
		}

		close(fd);
	}

	cleanup();
	return 0;
}

/***************************************************************
 * setup() - performs all ONE TIME setup for this test.
 ***************************************************************/
void setup(void)
{
	/* capture signals */
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	/* Pause if that option was specified */
	TEST_PAUSE;

	/* Create temp directory and change to that */
	tst_tmpdir();
}				/* End setup() */

/***************************************************************
 * cleanup() - performs all ONE TIME cleanup for this test at
 *		completion or premature exit.
 ***************************************************************/
void cleanup(void)
{
	/*
	 * print timing stats if that option was specified.
	 * print errno log if that option was specified.
	 */
	TEST_CLEANUP;

	/* Remove temp directory and files */
	tst_rmdir();

	/* exit with return code appropriate for results */
	tst_exit();

}				/* End cleanup() */

/***************************************************************
 * check_and_print(advice) - checks the return value
 *		of the previous madvise call
 *		and based on the advice value
 *		prints the appropriate messages.
 ***************************************************************/
void check_and_print(char *advice)
{
	if (TEST_RETURN == -1) {
		tst_resm(TFAIL,
			 "madvise test for %s failed with "
			 "return = %ld, errno = %d : %s",
			 advice, TEST_RETURN, TEST_ERRNO, strerror(TEST_ERRNO));
	} else if (STD_FUNCTIONAL_TEST) {
		tst_resm(TPASS, "madvise test for %s PASSED", advice);
	}
}
