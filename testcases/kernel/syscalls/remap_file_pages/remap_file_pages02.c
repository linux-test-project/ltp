/*
 * Copyright (C) Ricardo Salveti de Araujo, 2007
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * NAME
 *     remap_file_pages02
 *
 * DESCRIPTION
 *     The remap_file_pages() system call is used to create a non-linear
 *     mapping, that is, a mapping in which the pages of the file are mapped
 *     into a non-sequential order in memory.  The advantage of using
 *     remap_file_pages() over using repeated calls to mmap(2) is that
 *     the former  approach  does  not require the kernel to create
 *     additional VMA (Virtual Memory Area) data structures.
 *
 *     Runs remap_file_pages with wrong values and see if got the expected error
 *
 *     Setup:
 *       1. Global:
 *       2. Create a file, do a normal mmap with MAP_SHARED flag
 *
 *     Test:
 *       1. Test with a valid mmap but without MAP_SHARED flag
 *       2. Test with a invalid start argument
 *       3. Test with a invalid size argument
 *       4. Test with a invalid prot argument
 *
 *     Cleanup:
 *       Remove the file and erase the tmp directory
 *
 * Usage:  <for command-line>
 *  remap_file_pages02 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *
 *     02/11/2008 - Removed the pgoff test case, as the latest kernels doesn't
 *     verify the page offset (http://lkml.org/lkml/2007/11/29/325) - Ricardo
 *     Salveti de Araujo, <rsalvetidev@gmail.com>
 *
 *     19/10/2007 - Created by Ricardo Salveti de Araujo, <rsalvetidev@gmail.com>
 */

#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <linux/unistd.h>

#include "test.h"		/*LTP Specific Include File */

/* Test case defines */
#define WINDOW_START 0x48000000

static int page_sz;
size_t page_words;
size_t cache_pages;
size_t cache_sz;
size_t window_pages;
size_t window_sz;

static void setup();
static int setup01(int test);
static int setup02(int test);
static int setup03(int test);
static int setup04(int test);
static void cleanup();

char *TCID = "remap_file_pages02";
int TST_TOTAL = 4;

static char *cache_contents;
int fd;				/* File descriptor used at the test */
char *data = NULL;
char *data01 = NULL;

static struct test_case_t {
	char *err_desc;		/* Error description */
	int exp_errno;		/* Expected error number */
	char *exp_errval;	/* Expected error value string */
	int (*setupfunc) (int);	/* Test setup function */
	int (*cleanfunc) (int);	/* Test clean function */
	void *start;		/* Start argument */
	size_t size;		/* Size argument */
	int prot;		/* Prot argument */
	ssize_t pgoff;		/* Pgoff argument */
	int flags;		/* Flags argument */
} testcase[] = {
	{
	"start does not refer to a valid mapping created with the "
		    "MAP_SHARED flag", EINVAL, "EINVAL", setup01, NULL,
		    NULL, 0, 0, 2, 0}, {
	"start is invalid", EINVAL, "EINVAL", setup02, NULL, NULL, 0, 0, 2, 0},
	{
	"size is invalid", EINVAL, "EINVAL", setup03, NULL, NULL, 0, 0, 0, 0},
	{
	"prot is invalid", EINVAL, "EINVAL", setup04, NULL, NULL, 0, 0,
		    2, 0}
};

int main(int ac, char **av)
{
	int lc, i;

#if defined (__s390__) || (__s390x__) || (__ia64__)
	/* Disables the test in case the kernel version is lower than 2.6.12 and arch is s390 */
	if ((tst_kvercmp(2, 6, 12)) < 0) {
		tst_resm(TWARN,
			 "This test can only run on kernels that are 2.6.12 and higher");
		exit(0);
	}
#endif

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {

		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++) {
			/* do the setup if the test have one */
			if (testcase[i].setupfunc
			    && testcase[i].setupfunc(i) == -1) {
				tst_resm(TWARN,
					 "Failed to setup test %d"
					 " Skipping test", i);
				continue;
			}

			/* run the test */
			TEST(remap_file_pages
			     (testcase[i].start, testcase[i].size,
			      testcase[i].prot, testcase[i].pgoff,
			      testcase[i].flags));

			/* do the cleanup if the test have one */
			if (testcase[i].cleanfunc
			    && testcase[i].cleanfunc(i) == -1) {
				tst_brkm(TBROK, cleanup,
					 "Failed to cleanup test %d,"
					 " quitting the test", i);
			}

			/* verify the return code */
			if ((TEST_RETURN == -1)
			    && (TEST_ERRNO == testcase[i].exp_errno)) {
				tst_resm(TPASS,
					 "remap_file_pages(2) expected failure;"
					 " Got errno - %s : %s",
					 testcase[i].exp_errval,
					 testcase[i].err_desc);
			} else {
				tst_resm(TFAIL,
					 "remap_file_pages(2) failed to produce"
					 " expected error: %d, errno: %s."
					 " because got error %d",
					 testcase[i].exp_errno,
					 testcase[i].exp_errval, TEST_ERRNO);
			}
		}		/* end of test loops */
	}			/* end of  test looping */

	/* clean up and exit */
	cleanup();

	tst_exit();
}

/*
 * setup01() - create a mmap area without MAP_SHARED flag
 * - it uses the fd created at the main setup function
 */
int setup01(int test)
{
	data01 = mmap(NULL, cache_sz, PROT_READ | PROT_WRITE,
		      MAP_PRIVATE, fd, 0);

	if (data01 == MAP_FAILED) {
		tst_resm(TWARN, "mmap Error, errno=%d : %s", errno,
			 strerror(errno));
		return -1;
	}

	/* set up the test case struct for this test */
	testcase[test].start = data01;
	testcase[test].size = page_sz;

	return 0;
}

/*
 * setup02() - start is invalid
 */
int setup02(int test)
{
	/* set up the test case struct for this test */
	testcase[test].start = data + cache_sz;
	testcase[test].size = page_sz;

	return 0;
}

/*
 * setup03() - size is invalid
 */
int setup03(int test)
{
	/* set up the test case struct for this test */
	testcase[test].start = data;
	testcase[test].size = cache_sz + page_sz;

	return 0;
}

/*
 * setup04() - prot is invalid
 */
int setup04(int test)
{
	/* set up the test case struct for this test */
	testcase[test].start = data;
	testcase[test].size = page_sz;
	testcase[test].prot = -1;

	return 0;
}

/*
 * setup() - performs all ONE TIME setup for this test
 * - creates a defaul mmaped area to be able to run remap_file_pages
 */
void setup(void)
{
	int i, j;

	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	/* Get page size */
	if ((page_sz = getpagesize()) < 0) {
		tst_brkm(TFAIL, cleanup,
			 "getpagesize() fails to get system page size");
	}

	page_words = (page_sz / sizeof(char));

	/* Set the cache size */
	cache_pages = 32;
	cache_sz = cache_pages * page_sz;
	cache_contents = malloc(cache_sz * sizeof(char));

	for (i = 0; i < cache_pages; i++) {
		char *page = cache_contents + i * page_sz;

		for (j = 0; j < page_words; j++)
			page[j] = i;
	}

	if ((fd = open("cache", O_RDWR | O_CREAT | O_TRUNC, S_IRWXU)) < 0) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT|O_TRUNC,S_IRWXU) Failed, errno=%d : %s",
			 "cache", errno, strerror(errno));
	}

	if (write(fd, cache_contents, cache_sz) != cache_sz) {
		tst_resm(TFAIL,
			 "Write Error for \"cache_contents\" to \"cache_sz\" of %zu (errno=%d : %s)",
			 cache_sz, errno, strerror(errno));
		cleanup();
	}

	data = mmap((void *)WINDOW_START,
		    cache_sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (data == MAP_FAILED) {
		tst_resm(TFAIL, "mmap Error, errno=%d : %s", errno,
			 strerror(errno));
		cleanup();
	}

}

/*
* cleanup() - Performs one time cleanup for this test at
* completion or premature exit
*/
void cleanup(void)
{
	/* Close the file descriptor */
	close(fd);

	if (data)
		munmap(data, cache_sz);
	if (data01)
		munmap(data01, cache_sz);

	tst_rmdir();

}
