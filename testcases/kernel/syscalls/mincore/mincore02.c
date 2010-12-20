/*
 *
 *   Copyright (c) International Business Machines  Corp., 2001
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * NAME
 *	mincore02.c
 *
 * DESCRIPTION
 *	Testcase to check the error conditions for mincore
 *
 * ALGORITHM
 *	test1:
 *	This test case provides a functional validation for mincore system call.
 *      We mmap a file of known size (multiple of page size) and lock it in
 *      memory. Then we obtain page location information via mincore and compare
 *      the result with the expected value.
 *
 * USAGE:  <for command-line>
 *  ./mincore02
 *
 * HISTORY
 *  Author: Rajeev Tiwari: rajeevti@in.ibm.com
 *	08/2004 Rajeev Tiwari : Provides a functional validation of mincore system call.
 *
 * 	2004/09/10 Gernot Payer <gpayer@suse.de>
 * 		- Original testcase was based on wrong assumptions
 * 		- Major code cleanup
 *
 * RESTRICTIONS
 *	None
 */

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "test.h"
#include "usctest.h"

/* comment out if you need to debug */
/* #define DEBUG_MODE 1 */

/* Extern Global Variables */
/* Global Variables */
char *TCID = "mincore02";	/* test program identifier. */
int TST_TOTAL = 1;		/* total number of tests in this file.   */

static int file_desc = 0;	/* this is for the file descriptor */
static char *position = NULL;
static int p_size;		/* page size obtained via getpagesize() */
static int num_pages = 4;	/* four pages are used in this test */
static unsigned char *vec = NULL;

#if !defined(UCLINUX)

static char tmpfilename[] = "fooXXXXXX";

/* Extern Global Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    cleanup                                                       */
/*                                                                            */
/* Description: Performs all one time clean up for this test on successful    */
/*              completion,  premature exit or  failure. Closes all temporary */
/*              files, removes all temporary directories exits the test with  */
/*              appropriate return code by calling tst_exit() function.       */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits calling tst_exit(). Non '0' return code.   */
/*              On success - Exits calling tst_exit(). With '0' return code.  */
/*                                                                            */
/******************************************************************************/
void cleanup()
{

	/* Close all open file descriptors. */

	free(vec);
	munlock(position, p_size * num_pages);
	munmap(position, p_size * num_pages);
	TEST_CLEANUP;
	close(file_desc);
	remove(tmpfilename);

}

/* Local  Functions */
/******************************************************************************/
/*                                                                            */
/* Function:    setup                                                         */
/*                                                                            */
/* Description: Performs all one time setup for this test. This function is   */
/*              typically used to capture signals, create temporary dirs      */
/*              and temporary files that may be used in the course of this    */
/*              test.                                                         */
/*                                                                            */
/* Input:       None.                                                         */
/*                                                                            */
/* Output:      None.                                                         */
/*                                                                            */
/* Return:      On failure - Exits by calling cleanup().                      */
/*                                                     */
/*                                                                            */
/******************************************************************************/

void setup()
{
	char *buf;
	int size;
	int status;

	if (-1 == (p_size = getpagesize())) {
		tst_brkm(TBROK, cleanup, "Unable to get page size: %s",
			 strerror(errno));
	}

	size = p_size * num_pages;
	buf = (char *)malloc(p_size * num_pages);

	memset(buf, 42, size);
	vec = malloc((size + p_size - 1) / p_size);

	/* create a temporary file to be used */

	if (-1 == (file_desc = mkstemp(tmpfilename))) {
		tst_brkm(TBROK, cleanup, "Unable to create temporary file: %s",
			 strerror(errno));
	}

	/* fill the temporary file with two pages of data */

	if (-1 == write(file_desc, buf, size)) {
		tst_brkm(TBROK, cleanup, "Error in writing to the file: %s",
			 strerror(errno));
	}
	free(buf);

	/* mmap the file in virtual address space in read , write and execute mode , the mapping should be shared  */

	if (MAP_FAILED ==
	    (position =
	     (char *)mmap(0, size, PROT_READ | PROT_WRITE | PROT_EXEC,
			  MAP_SHARED, file_desc, 0))) {
		tst_brkm(TBROK, cleanup,
			 "Unable to map file for read/write.  Error: %d (%s)",
			 errno, strerror(errno));
	}

	/* lock mmapped file, so mincore returns "in core" for all pages */
	if ((status = mlock(position, size)) == -1) {
		tst_brkm(TBROK, cleanup, "Unable to lock the file: %s",
			 strerror(errno));
	}
	return;
}

int main(int argc, char **argv)
{
	int lock_pages, counter;

	setup();

	if (-1 == mincore((void *)position, num_pages * p_size, vec)) {
		tst_brkm(TBROK, cleanup,
			 "Unable to execute mincore system call: %s",
			 strerror(errno));
	}

	/* check status of pages */

	lock_pages = 0;

	for (counter = 0; counter < num_pages; counter++) {
		if (vec[counter] & 1)
			lock_pages++;
	}

	if (lock_pages == num_pages)
		tst_resm(TPASS, "%d pages locked, %d pages in-core", num_pages,
			 lock_pages);
	else
		tst_resm(TFAIL,
			 "not all locked pages are in-core: no. locked: %d, no. in-core: %d",
			 num_pages, lock_pages);

	cleanup();
	tst_exit();
}

#else

int main()
{
	tst_resm(TINFO, "test is not available on uClinux");
	tst_exit();
}

#endif /* if !defined(UCLINUX) */