/***************************************************************************
 *            madvise2.c
 *
 *  Fri May 14 17:23:19 2004
 *	Copyright (c) International Business Machines  Corp., 2004
 *  Email	sumit@in.ibm.com
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
 *    TEST CASES
 *
 *	1.) madvise(2) error conditions...(See Description)
 *
 *	INPUT SPECIFICATIONS
 *		The standard options for system call tests are accepted.
 *		(See the parse_opts(3) man page).
 *
 *	OUTPUT SPECIFICATIONS
 *		Output describing whether test cases passed or failed.
 *
 *	ENVIRONMENTAL NEEDS
 *		None
 *
 *	SPECIAL PROCEDURAL REQUIREMENTS
 *		None
 *
 *	DETAILED DESCRIPTION
 *		This is a test for the madvise(2) system call. It is intended
 *		to provide a complete exposure of the system call. It tests
 *		madvise(2) for all error conditions to occur correctly.
 *
 *		(A) Test Case for EINVAL
 *		1. start is not page-aligned
 *		2. advice is not a valid value
 *		3. application is attempting to release
 *			   locked or shared pages (with MADV_DONTNEED)
 *
 *		(B) Test Case for ENOMEM
 *		4. addresses in the specified range are not currently mapped
 *			   or are outside the address space of the process
 *			b. Not enough memory - paging in failed
 *
 *		(C) Test Case for EBADF
 *		5. the map exists,
 *			   but the area maps something that isn't a file.
 *
 *		(D) Test Case for EAGAIN
 *		6. a kernel resource was temporarily unavailable.
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
#include <sys/types.h>		/* For fstat */
#include <sys/stat.h>		/* For fstat */
#include <unistd.h>		/* For fstat & getpagesize() */
#include <sys/mman.h>		/* For madvise */
#include <fcntl.h>
#include <sys/time.h>		/* For rlimit */
#include <sys/resource.h>	/* For rlimit */

#include "test.h"
#include "usctest.h"

/* Uncomment the following line in DEBUG mode */
//#define MM_DEBUG 1
#define MM_RLIMIT_RSS 5

static void setup(void);
static void cleanup(void);
static void check_and_print(int expected_errno);

char *TCID = "madvise02";
int TST_TOTAL = 7;

int main(int argc, char *argv[])
{
	int lc, fd, pagesize;
	int i;
	unsigned long len;
	char *file, *low, *high;
	struct stat stat;
	char *ptr_memory_allocated = NULL;
	char *tmp_memory_allocated = NULL;

	char *msg = NULL;
	char filename[64];
	char *progname = NULL;
	char *str_for_file = "abcdefghijklmnopqrstuvwxyz12345\n";

	msg = parse_opts(argc, argv, NULL, NULL);
	if (msg)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	progname = *argv;
	sprintf(filename, "%s-out.%d", progname, getpid());

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		fd = open(filename, O_RDWR|O_CREAT, 0664);
		if (fd < 0)
			tst_brkm(TBROK, cleanup, "open failed");
#ifdef MM_DEBUG
		tst_resm(TINFO, "filename = %s opened successfully", filename);
#endif

		pagesize = getpagesize();

		/* Writing 16 pages of random data into this file */
		for (i = 0; i < (pagesize / 2); i++)
			if (write(fd, str_for_file, strlen(str_for_file)) == -1)
				tst_brkm(TBROK|TERRNO, cleanup, "write failed");

		if (fstat(fd, &stat) == -1)
			tst_brkm(TBROK, cleanup, "fstat failed");

		file = mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
		if (file == MAP_FAILED)
			tst_brkm(TBROK|TERRNO, cleanup, "mmap failed");
#ifdef MM_DEBUG
		tst_resm(TINFO, "The Page size is %d", pagesize);
#endif

		/* Test Case 1 */
		TEST(madvise(file + 100, stat.st_size, MADV_NORMAL));
		check_and_print(EINVAL);

		/* Test Case 2 */
		TEST(madvise(file, stat.st_size, 1212));
		check_and_print(EINVAL);

#if !defined(UCLINUX)
		/* Test Case 3 */
		if (mlock((void *)file, stat.st_size) < 0)
			tst_brkm(TBROK, cleanup, "mlock failed");

		TEST(madvise(file, stat.st_size, MADV_DONTNEED));
		check_and_print(EINVAL);

		if (munmap(file, stat.st_size) == -1)
			tst_brkm(TBROK|TERRNO, cleanup, "munmap failed");
#endif /* if !defined(UCLINUX) */

		/* Test Case 4 */

		/* We cannot be sure, which region is mapped, which is
		 * not, at runtime.
		 * So, we will create two maps(of the same file),
		 * unmap the map at higher address.
		 * Now issue an madvise() on a region covering the
		 * region which we unmapped.
		 */

		low = mmap(NULL, stat.st_size / 2, PROT_READ, MAP_SHARED,
			   fd, 0);
		if (low == MAP_FAILED)
			tst_brkm(TBROK, cleanup, "mmap [low] failed");

		high = mmap(NULL, stat.st_size / 2, PROT_READ, MAP_SHARED,
			    fd, stat.st_size / 2);
		if (high == MAP_FAILED)
			tst_brkm(TBROK, cleanup, "mmap [high] failed");

		/* Swap if necessary to make low < high */
		if (low > high) {
			char *tmp;
			tmp = high;
			high = low;
			low = tmp;
		}

		len = (high - low) + pagesize;

		if (munmap(high, stat.st_size / 2) < 0)
			tst_brkm(TBROK|TERRNO, cleanup, "munmap [high] failed");

		TEST(madvise(low, len, MADV_NORMAL));
		check_and_print(ENOMEM);

		/* Test Case 5 */
		/* Unmap the file map from low */
		if (munmap(low, stat.st_size / 2) < 0)
			tst_brkm(TBROK|TERRNO, cleanup, "munmap [low] failed");
		/* Create one memory segment using malloc */
		ptr_memory_allocated = (char *)malloc(5 * pagesize);
		/*
		 * Take temporary pointer for later use, freeing up the
		 * original one.
		 */
		tmp_memory_allocated = ptr_memory_allocated;
		tmp_memory_allocated =
			(char *)(((unsigned long)tmp_memory_allocated +
				pagesize - 1) & ~(pagesize - 1));

		TEST(madvise
		     (tmp_memory_allocated, 5 * pagesize, MADV_WILLNEED));
		check_and_print(EBADF);
		free((void *)ptr_memory_allocated);

		close(fd);
	}
	cleanup();
	tst_exit();
}

static void setup(void)
{

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();
}

static void cleanup(void)
{
	TEST_CLEANUP;

	tst_rmdir();

}

static void check_and_print(int expected_errno)
{
	if (TEST_RETURN == -1) {
		if (TEST_ERRNO == expected_errno)
			tst_resm(TPASS|TTERRNO, "failed as expected");
		else
			tst_resm(TFAIL|TTERRNO,
			    "failed unexpectedly; expected - %d : %s",
			    expected_errno, strerror(expected_errno));
	} else {
		tst_resm(TFAIL, "madvise succeeded unexpectedly");
	}
}
