/******************************************************************************
 *			 fallocate02.c
 *	Mon Dec 24 2007
 *	Copyright (c) International Business Machines  Corp., 2007
 *	Emali : sharyathi@in.ibm.com
 ******************************************************************************/

/***************************************************************************
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
***************************************************************************/

/*****************************************************************************
 *
 *	OS Test - International Business Machines Corp. 2007.
 *
 *	TEST IDENTIFIER	: fallocate02
 *
 *	EXECUTED BY		: anyone
 *
 *	TEST TITLE		: Checks for Errors from fallocate()
 *
 *	TEST CASE TOTAL	: 7
 *
 *	CPU ARCHITECTURES	: PPC,X86, X86_64
 *
 *	AUTHOR			: Sharyathi Nagesh
 *
 *	CO-PILOT			:
 *
 *	DATE STARTED		: 24/12/2007
 *
 *	TEST CASES
 *	(Tests fallocate() for different test cases as reported in map page)
 *
 *	INPUT SPECIFICATIONS
 *		No input needs to be specified
 *		  fallocate() in-puts are specified through test_data
 *
 *	OUTPUT SPECIFICATIONS
 *		fallocate Error message matches with the expected error message.
 *
 *	ENVIRONMENTAL NEEDS
 *		Test Needs to be executed on file system supporting ext4
 *   LTP {TMP} Needs to be set to such a folder
 *
 *	SPECIAL PROCEDURAL REQUIREMENTS
 *		None
 *
 *	DETAILED DESCRIPTION
 *		This is a test case for fallocate() system call.
 *		This test suite tests various error messages from fallocate
 *		If the error message received matches with the expected
 *		test is considered passed else test fails
 *		Provided TEST_DEFAULT to switch b/w modes
 *
 *		Total 7 Test Cases :-
 *		Various error messages from the man page
 *
 *	Setup:
 *		Setup files on which fallocate is to be called
 *
 *	Test:
 *		Loop if the proper options are given.
 *		Execute system call
 *		Check return code.
 *		If error obtained matches with the expected error
 *		PASS the test, otherwise TEST FAILS
 *		Provided TEST_DEFAULT to switch b/w modes
 *
 *	Cleanup:
 *		Cleanup the temporary folder
 *
*************************************************************************/

/* Standard Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <endian.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/utsname.h>

/* Harness Specific Include Files. */
#include "test.h"
#include "usctest.h"
#include "linux_syscall_numbers.h"

#define BLOCKS_WRITTEN 12

#ifdef TEST_DEFAULT
#define DEFAULT_TEST_MODE 0	//DEFAULT MODE
#else
#define DEFAULT_TEST_MODE 1	//FALLOC_FL_KEEP_SIZE MODE
#endif

#define OFFSET 12

/*Local Functions*/
static inline long fallocate();
void populate_file();
void create_fifo();
void create_pipe();
void get_blocksize(int fd);

/* Extern Global Variables */
/* Global Variables */
char *TCID = "fallocate02";	/* test program identifier */
char fnamew[255];		/* Files used for testing */
char fnamer[255];		/* Files used for testing */
int fdw;
int fdr;
enum { RFILE, WFILE, PIPE, FIFO };
struct test_data_t {
	int file;
	int mode;
	loff_t offset;
	loff_t len;
	int error;
} test_data[] = {
	{
	RFILE, DEFAULT_TEST_MODE, 0, 1, EBADF}, {
	WFILE, DEFAULT_TEST_MODE, -1, 1, EINVAL}, {
	WFILE, DEFAULT_TEST_MODE, 1, -1, EINVAL}, {
	WFILE, DEFAULT_TEST_MODE, BLOCKS_WRITTEN, 0, EINVAL}, {
	WFILE, DEFAULT_TEST_MODE, BLOCKS_WRITTEN, -1, EINVAL}, {
	WFILE, DEFAULT_TEST_MODE, -(BLOCKS_WRITTEN + OFFSET), 1, EINVAL}, {
	WFILE, DEFAULT_TEST_MODE, BLOCKS_WRITTEN - OFFSET, 1, 0}
};
/* total number of tests in this file */
int TST_TOTAL = sizeof(test_data) / sizeof(test_data[0]);
int block_size;
int buf_size;

/******************************************************************************
 * Performs all one time clean up for this test on successful
 * completion,  premature exit or  failure. Closes all temporary
 * files, removes all temporary directories exits the test with
 * appropriate return code by calling tst_exit() function.
******************************************************************************/
extern void cleanup()
{
	/* Close all open file descriptors. */
	if (close(fdw) == -1)
		tst_resm(TWARN|TERRNO, "close(%s) failed", fnamew);

	if (close(fdr) == -1)
		tst_resm(TWARN|TERRNO, "close(%s) failed", fnamer);

	tst_rmdir();

}

/*****************************************************************************
 * Performs all one time setup for this test. This function is
 * used to create temporary dirs and temporary files
 * that may be used in the course of this test
 ******************************************************************************/
void setup()
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	sprintf(fnamer, "tfile_read_%d", getpid());
	sprintf(fnamew, "tfile_write_%d", getpid());

	fdr = open(fnamer, O_RDONLY | O_CREAT, S_IRUSR);
	if (fdr == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "open(%s, O_RDONLY|O_CREAT, S_IRUSR) failed",
			 fnamer);
	fdw = open(fnamew, O_RDWR | O_CREAT, S_IRWXU);
	if (fdw == -1)
		tst_brkm(TBROK|TERRNO, cleanup,
			 "open(%s, O_RDWR|O_CREAT, S_IRWXU) failed",
			 fnamew);
	get_blocksize(fdr);
	populate_file();
}

/*****************************************************************************
 * Gets the block size for the file system
 ******************************************************************************/
void get_blocksize(int fd)
{
	struct stat file_stat;

	if (fstat(fd, &file_stat) < 0)
		tst_resm(TFAIL|TERRNO,
			 "fstat failed while getting block_size");

	block_size = (int)file_stat.st_blksize;
	buf_size = block_size;
}

/*****************************************************************************
 * Writes data into the file
 ******************************************************************************/
void populate_file()
{
	char buf[buf_size + 1];
	int index;
	int blocks;
	int data;
	for (blocks = 0; blocks < BLOCKS_WRITTEN; blocks++) {
		for (index = 0; index < buf_size; index++)
			buf[index] = 'A' + (index % 26);
		buf[buf_size] = '\0';
		if ((data = write(fdw, buf, buf_size)) < 0)
			tst_brkm(TBROK|TERRNO, cleanup,
				 "Unable to write to %s", fnamew);
	}
}

/*****************************************************************************
 * Wraper function to call fallocate system call
 ******************************************************************************/
static inline long fallocate(int fd, int mode, loff_t offset, loff_t len)
{
#if __WORDSIZE == 32
	return (long) syscall(__NR_fallocate, fd, mode,
	    __LONG_LONG_PAIR((off_t)(offset >> 32), (off_t)offset),
	    __LONG_LONG_PAIR((off_t)(len >> 32), (off_t)len));
#else
	return syscall(__NR_fallocate, fd, mode, offset, len);
#endif
}

/*****************************************************************************
 * Main function that calls the system call with the  appropriate parameters
 ******************************************************************************/
/* ac: number of command line parameters */
/* av: pointer to the array of the command line parameters */
int main(int ac, char **av)
{

	int test_index = 0;
	int lc;
	int fd;
	char fname[255], *msg;

	/***************************************************************
	     * parse standard options
     	***************************************************************/
	if ((msg = parse_opts(ac, av, NULL, NULL)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	/* perform global test setup, call setup() function. */
	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		Tst_count = 0;
		for (test_index = 0; test_index < TST_TOTAL; test_index++) {
			switch (test_data[test_index].file) {
			case RFILE:
				fd = fdr;
				strcpy(fname, fnamer);
				break;
			case WFILE:
				fd = fdw;
				strcpy(fname, fnamew);
				break;
			default:
				tst_brkm(TCONF, cleanup, "invalid test setting");
				tst_exit();
			}

			TEST(fallocate
			     (fd, test_data[test_index].mode,
			      test_data[test_index].offset * block_size,
			      test_data[test_index].len * block_size));
			/* check return code */
			if (TEST_ERRNO != test_data[test_index].error) {
				if (TEST_ERRNO == EOPNOTSUPP || TEST_ERRNO == ENOSYS) {
					tst_brkm(TCONF, cleanup,
						 "fallocate system call is not implemented");
				}
				TEST_ERROR_LOG(TEST_ERRNO);
				tst_resm(TFAIL|TTERRNO, "fallocate(%s:%d, %d, %"PRId64", %"PRId64") failed, expected errno:%d",
					fname, fd, test_data[test_index].mode,
					test_data[test_index].offset * block_size,
					test_data[test_index].len * block_size,
					test_data[test_index].error);
			} else {
				/* No Verification test, yet... */
				tst_resm(TPASS,
					 "fallocate(%s:%d, %d, %"PRId64", %"PRId64") returned %d",
					 fname, fd, test_data[test_index].mode,
					 test_data[test_index].offset *
					 block_size,
					 test_data[test_index].len * block_size,
					 TEST_ERRNO);
			}
		}
	}
	cleanup();
	tst_exit();
}
