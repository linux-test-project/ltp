/******************************************************************************
 *				 fallocate01.c
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
 *	TEST IDENTIFIER	: fallocate01
 *
 *	EXECUTED BY		: anyone
 *
 *	TEST TITLE		: Basic test for fallocate()
 *
 *	TEST CASE TOTAL	: 2
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
 *	(Working of fallocate under 2 modes)
 *	 1) DEFAULT 2)FALLOC_FL_KEEP_SIZE
 *
 *	INPUT SPECIFICATIONS
 *		No input needs to be specified
 *		  fallocate() in puts are generated randomly
 *
 *	OUTPUT SPECIFICATIONS
 *		Output describing whether test cases passed or failed.
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
 *		This test suite tests basic working of fallocate under different modes
 *		It trys to fallocate memory blocks and write into that block
 *
 *		Total 2 Test Cases :-
 *		(1) Test Case for DEFAULT MODE
 *		(2) Test Case for FALLOC_FL_KEEP_SIZE
 *
 *	Setup:
 *		Setup file on which fallocate is to be called
 *		Set up 2 files for each mode
 *
 *	Test:
 *		Loop if the proper options are given.
 *		Execute system call
 *		Check return code, if system call did fail
 *		lseek to some random location with in allocate block
 *		write data into the locattion Report if any error encountered
 *		PASS the test otherwise
 *
 *	Cleanup:
 *		Cleanup the temporary folder
 *
*************************************************************************/
/*
 * Patch Description:
    Test Failure reason in SGX-LKL:
    Test is happening on temporary file created under /tmp, on which fallocate system call is failing.

 * Workaround to fix the issue:
    fallocate system call is failing on temporary file created under /tmp directory.
    So modified the tests to create directory in root filesystem.
    TODO: Enable tst_tmpdir once git issue 734 is fixed
    Issue: [Tests] fallocate system call is failing on /tmp filesystem.
    https://github.com/lsds/sgx-lkl/issues/734
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <endian.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/utsname.h>

#include "test.h"
#include "safe_macros.h"
#include "lapi/fallocate.h"
#include "lapi/fcntl.h"

#define BLOCKS_WRITTEN 12
#define tempdir "tempdir"
#define fname_mode1 tempdir "/tfile_mode1"	/* Files used for testing */
#define fname_mode2 tempdir "/tfile_mode2"	/* Files used for testing */

void get_blocksize(int);
void populate_files(int fd);
void runtest(int, int, loff_t);

char *TCID = "fallocate01";
int fd_mode1, fd_mode2;
int TST_TOTAL = 2;
loff_t block_size;
int buf_size;

/******************************************************************************
 * Performs all one time clean up for this test on successful
 * completion,  premature exit or  failure. Closes all temporary
 * files, removes all temporary directories exits the test with
 * appropriate return code by calling tst_exit() function.
******************************************************************************/
void cleanup(void)
{

	if (close(fd_mode1) == -1)
		tst_resm(TWARN | TERRNO, "close(%s) failed", fname_mode1);
	if (close(fd_mode2) == -1)
		tst_resm(TWARN | TERRNO, "close(%s) failed", fname_mode2);
	// fallocate systemcall is failing on temporary directory created under /tmp
	// Hence test directory is created in root filesystem.
	remove(fname_mode1);
      remove(fname_mode2);
      rmdir(tempdir); // TODO: Revert changes once (https://github.com/lsds/sgx-lkl/issues/734) is fixed
}

/*****************************************************************************
 * Performs all one time setup for this test. This function is
 * used to create temporary dirs and temporary files
 * that may be used in the course of this test
 ******************************************************************************/
void setup(void)
{
	/* Create temporary directories */
	TEST_PAUSE;
	// fallocate system call is failing on temporary directory under /tmp using tst_tmpdir function.
	// so, creating test directory in root filesystem as workaroud
	mkdir(tempdir, 0777); // TODO: Revert changes once (https://github.com/lsds/sgx-lkl/issues/734) is fixed

	fd_mode1 = SAFE_OPEN(cleanup, fname_mode1, O_RDWR | O_CREAT, 0700);
	get_blocksize(fd_mode1);
	populate_files(fd_mode1);

	fd_mode2 = SAFE_OPEN(cleanup, fname_mode2, O_RDWR | O_CREAT, 0700);
	populate_files(fd_mode2);
}

/*****************************************************************************
 * Gets the block size for the file system
 ******************************************************************************/
void get_blocksize(int fd)
{
	struct stat file_stat;

	if (fstat(fd, &file_stat) < 0)
		tst_resm(TFAIL | TERRNO,
			 "fstat failed while getting block_size");

	block_size = file_stat.st_blksize;
	buf_size = block_size;
}

/*****************************************************************************
 * Writes data into the file
 ******************************************************************************/

void populate_files(int fd)
{
	char buf[buf_size + 1];
	int index;
	int blocks;
	int data;

	for (blocks = 0; blocks < BLOCKS_WRITTEN; blocks++) {
		for (index = 0; index < buf_size; index++)
			buf[index] = 'A' + (index % 26);
		buf[buf_size] = '\0';
		if ((data = write(fd, buf, buf_size)) == -1)
			tst_brkm(TBROK | TERRNO, cleanup, "write failed");
	}
}

int main(int ac, char **av)
{
	loff_t expected_size;
	int lc;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		expected_size = BLOCKS_WRITTEN * block_size + block_size;
		runtest(0, fd_mode1, expected_size);

		expected_size = BLOCKS_WRITTEN * block_size;
		runtest(FALLOC_FL_KEEP_SIZE, fd_mode2, expected_size);
	}

	cleanup();
	tst_exit();
}

/*****************************************************************************
 * Calls the system call, with appropriate parameters and writes data
 ******************************************************************************/
void runtest(int mode, int fd, loff_t expected_size)
{
	loff_t offset;
	loff_t len = block_size;
	loff_t write_offset, lseek_offset;
	offset = lseek(fd, 0, SEEK_END);
	struct stat file_stat;
	errno = 0;

	TEST(fallocate(fd, mode, offset, len));
	/* check return code */
	if (TEST_RETURN != 0) {
		if (TEST_ERRNO == EOPNOTSUPP || TEST_ERRNO == ENOSYS) {
			tst_brkm(TCONF, cleanup,
				 "fallocate system call is not implemented");
		}
		tst_resm(TFAIL | TTERRNO,
			 "fallocate(%d, %d, %" PRId64 ", %" PRId64 ") failed",
			 fd, mode, offset, len);
		return;
	} else {
		tst_resm(TPASS,
			 "fallocate(%d, %d, %" PRId64 ", %" PRId64
			 ") returned %ld", fd, mode, offset, len,
			 TEST_RETURN);
	}

	if (fstat(fd, &file_stat) < 0)
		tst_resm(TFAIL | TERRNO, "fstat failed after fallocate()");

	if (file_stat.st_size != expected_size)
		tst_resm(TFAIL | TTERRNO,
			 "fstat test fails on fallocate (%d, %d, %" PRId64 ", %"
			 PRId64 ") Failed on mode", fd, mode, offset, len);

	write_offset = random() % len;
	lseek_offset = lseek(fd, write_offset, SEEK_CUR);
	if (lseek_offset != offset + write_offset) {
		tst_resm(TFAIL | TTERRNO,
			 "lseek fails in fallocate(%d, %d, %" PRId64 ", %"
			 PRId64 ") failed on mode", fd, mode, offset, len);
		return;
	}
	//Write a character to file at random location
	TEST(write(fd, "A", 1));
	/* check return code */
	if (TEST_RETURN == -1) {
		tst_resm(TFAIL | TTERRNO,
			 "write fails in fallocate(%d, %d, %" PRId64 ", %"
			 PRId64 ") failed", fd, mode, offset, len);
	} else {
		tst_resm(TPASS,
			 "write operation on fallocated(%d, %d, %"
			 PRId64 ", %" PRId64 ") returned %ld", fd, mode,
			 offset, len, TEST_RETURN);
	}
}
