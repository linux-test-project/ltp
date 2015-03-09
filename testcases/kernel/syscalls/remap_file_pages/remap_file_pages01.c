/*
 * Copyright (C) Ingo Molnar, 2002
 * Copyright (C) Ricardo Salveti de Araujo, 2007
 * Copyright (C) International Business Machines  Corp., 2007
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
 *     remap_file_pages01
 *
 * DESCRIPTION
 *     The remap_file_pages() system call is used to create a non-linear
 *     mapping, that is, a mapping in which the pages of the file are mapped
 *     into a non-sequential order in memory.  The advantage of using
 *     remap_file_pages() over using repeated calls to mmap(2) is that
 *     the former  approach  does  not require the kernel to create
 *     additional VMA (Virtual Memory Area) data structures.
 *
 *     Runs remap_file_pages agains a mmaped area and check the results
 *
 *     Setup:
 *       Create a temp directory, open a file and get the file descriptor
 *
 *     Test:
 *       Test with a normal file and with /dev/shm/cache_<pid>
 *       1. Set up the cache
 *       2. Write the cache to the file
 *       3. Runs mmap at the same file
 *       4. Runs remap_file_pages at the mapped memory
 *       5. Check the results
 *   $
 *     Cleanup:
 *       Remove the file and erase the tmp directory
 *
 * Usage:  <for command-line>
 *  remap_file_pages01 [-c n] [-f] [-i n] [-I x] [-P x] [-t]
 *     where,  -c n : Run n copies concurrently.
 *             -f   : Turn off functionality Testing.
 *             -i n : Execute test n times.
 *             -I x : Execute test for x seconds.
 *             -P x : Pause for x seconds between iterations.
 *             -t   : Turn on syscall timing.
 *
 * HISTORY
 *                - Ingo Molnar, <mingo@elte.hu> wrote this test case
 *                - Nick Piggin, <nickpiggin@yahoo.com.au> did the following cleanup
 *
 *     11/10/2007 - Port to LTP format by Subrata Modak, <subrata@linux.vnet.ibm.com>
 *                  and Ricardo Salveti de Araujo, <rsalveti@linux.vnet.ibm.com>
 *     25/02/2008 - Renaud Lottiaux, <Renaud.Lottiaux@kerlabs.com>
 *                  Fix NFS remove tmpdir issue due to non unmapped files.
 *                  Fix concurrency issue on the file /dev/shm/cache.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/times.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
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
static void cleanup();
static void test_nonlinear(int fd);

char *TCID = "remap_file_pages01";
int TST_TOTAL = 2;

static char *cache_contents;
int fd1, fd2;			/* File descriptors used at the test */
char fname[255];

int main(int ac, char **av)
{
	int lc;

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

		test_nonlinear(fd1);
		tst_resm(TPASS, "Non-Linear shm file OK");

		test_nonlinear(fd2);
		tst_resm(TPASS, "Non-Linear /tmp/ file OK");
	}

	/* clean up and exit */
	cleanup();
	tst_exit();

}

/* test case function, that runs remap_file_pages */
static void test_nonlinear(int fd)
{
	char *data = NULL;
	int i, j, repeat = 2;

	for (i = 0; i < cache_pages; i++) {
		char *page = cache_contents + i * page_sz;

		for (j = 0; j < page_words; j++)
			page[j] = i;
	}

	if (write(fd, cache_contents, cache_sz) != cache_sz) {
		tst_resm(TFAIL,
			 "Write Error for \"cache_contents\" to \"cache_sz\" of %zu (errno=%d : %s)",
			 cache_sz, errno, strerror(errno));
		cleanup(NULL);
	}

	data = mmap((void *)WINDOW_START,
		    window_sz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if (data == MAP_FAILED) {
		tst_resm(TFAIL, "mmap Error, errno=%d : %s", errno,
			 strerror(errno));
		cleanup(NULL);
	}

again:
	for (i = 0; i < window_pages; i += 2) {
		char *page = data + i * page_sz;

		if (remap_file_pages(page, page_sz * 2, 0,
				     (window_pages - i - 2), 0) == -1) {
			tst_resm(TFAIL | TERRNO,
				 "remap_file_pages error for page=%p, "
				 "page_sz=%d, window_pages=%zu",
				 page, (page_sz * 2), (window_pages - i - 2));
			cleanup(data);
		}
	}

	for (i = 0; i < window_pages; i++) {
		/*
		 * Double-check the correctness of the mapping:
		 */
		if (i & 1) {
			if (data[i * page_sz] != window_pages - i) {
				tst_resm(TFAIL,
					 "hm, mapped incorrect data, "
					 "data[%d]=%d, (window_pages-%d)=%zu",
					 (i * page_sz), data[i * page_sz], i,
					 (window_pages - i));
				cleanup(data);
			}
		} else {
			if (data[i * page_sz] != window_pages - i - 2) {
				tst_resm(TFAIL,
					 "hm, mapped incorrect data, "
					 "data[%d]=%d, (window_pages-%d-2)=%zu",
					 (i * page_sz), data[i * page_sz], i,
					 (window_pages - i - 2));
				cleanup(data);
			}
		}
	}

	if (--repeat)
		goto again;

	munmap(data, window_sz);
}

/* setup() - performs all ONE TIME setup for this test */
void setup(void)
{

	tst_sig(FORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	TEST_PAUSE;

	/* Get page size */
	page_sz = getpagesize();

	page_words = page_sz;

	/* Set the cache size */
	cache_pages = 1024;
	cache_sz = cache_pages * page_sz;
	cache_contents = malloc(cache_sz * sizeof(char));

	/* Set the window size */
	window_pages = 16;
	window_sz = window_pages * page_sz;

	sprintf(fname, "/dev/shm/cache_%d", getpid());

	if ((fd1 = open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU)) < 0) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT|O_TRUNC,S_IRWXU) Failed, errno=%d : %s",
			 fname, errno, strerror(errno));
	}

	if ((fd2 = open("cache", O_RDWR | O_CREAT | O_TRUNC, S_IRWXU)) < 0) {
		tst_brkm(TBROK, cleanup,
			 "open(%s, O_RDWR|O_CREAT|O_TRUNC,S_IRWXU) Failed, errno=%d : %s",
			 "cache", errno, strerror(errno));
	}

}

/*
* cleanup() - Performs one time cleanup for this test at
* completion or premature exit
*/
void cleanup(char *data)
{
	/* Close the file descriptors */
	close(fd1);
	close(fd2);

	if (data)
		munmap(data, window_sz);

	/* Remove the /dev/shm/cache_<pid> file */
	unlink(fname);

	tst_rmdir();

}
