/*
 * mmap001.c - Tests mmapping a big file and writing it once
 *
 * Copyright (C) 2000 Juan Quintela <quintela@fi.udc.es>
 *                    Aaron Laffin <alaffin@sgi.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "test.h"
#include "usctest.h"

char *TCID = "mmap001";
int TST_TOTAL = 5;
extern int Tst_count;
static char *filename = NULL;
static int m_opt = 0;
static char *m_copt;

void cleanup()
{
	/*
	 * remove the tmp directory and exit
	 */

	if (filename)
		free(filename);

	TEST_CLEANUP;

	tst_rmdir();

	tst_exit();
}

void setup()
{
	char buf[1024];
	/*
	 * setup a default signal hander and a
	 * temporary working directory.
	 */
	tst_sig(FORK, DEF_HANDLER, cleanup);

	TEST_PAUSE;

	tst_tmpdir();

	snprintf(buf, 1024, "testfile.%d", getpid());

	filename = strdup(buf);
}

void help()
{
	printf("  -m x    size of mmap in pages (default 1000)\n");
}

/*
 * add the -m option whose parameter is the
 * pages that should be mapped.
 */
option_t options[] = {
	{"m:", &m_opt, &m_copt},
	{NULL, NULL, NULL}
};

int main(int argc, char *argv[])
{
	char *array, *msg;
	int i, lc;
	int fd;
	unsigned int pages, memsize;

	if ((msg = parse_opts(argc, argv, options, help)) != (char *)NULL)
		tst_brkm(TBROK, cleanup, "OPTION PARSING ERROR - %s", msg);

	if (m_opt) {
		memsize = pages = atoi(m_copt);

		if (memsize < 1) {
			tst_brkm(TBROK, cleanup, "Invalid arg for -m: %s",
				 m_copt);
		}

		memsize *= getpagesize();	/* N PAGES */

	} else {
		/*
		 * default size 1000 pages;
		 */
		memsize = pages = 1000;
		memsize *= getpagesize();
	}

	tst_resm(TINFO, "mmap()ing file of %u pages or %u bytes", pages,
		 memsize);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		Tst_count = 0;

		fd = open(filename, O_RDWR | O_CREAT, 0666);
		if ((fd == -1))
			tst_brkm(TBROK, cleanup, "Problems opening files");

		if (lseek(fd, memsize, SEEK_SET) != memsize) {
			close(fd);
			tst_brkm(TBROK, cleanup,
				 "Problems doing the lseek: %d: %s", errno,
				 strerror(errno));
		}

		if (write(fd, "\0", 1) != 1) {
			close(fd);
			tst_brkm(TBROK, cleanup, "Problems writing: %d: %s",
				 errno, strerror(errno));
		}

		array = mmap(0, memsize, PROT_WRITE, MAP_SHARED, fd, 0);
		if (array == (char *)MAP_FAILED) {
			tst_resm(TFAIL, "mmap() failed: %d: %s",
				 errno, strerror(errno));
			tst_exit();
		} else {
			tst_resm(TPASS, "mmap() completed successfully.");
		}

		if (STD_FUNCTIONAL_TEST) {
			tst_resm(TINFO, "touching mmaped memory");

			for (i = 0; i < memsize; i++) {
				array[i] = (char)i;
			}

			/*
			 * seems that if the map area was bad, we'd get SEGV, hence we can
			 * indicate a PASS.
			 */
			tst_resm(TPASS,
				 "we're still here, mmaped area must be good");

			TEST(msync(array, memsize, MS_SYNC));

			if (TEST_RETURN == -1) {
				tst_resm(TFAIL, "msync() failed: errno: %d: %s",
					 TEST_ERRNO, strerror(TEST_ERRNO));
			} else {
				tst_resm(TPASS, "msync() was successful");
			}

		}
		/* STD_FUNCTIONAL_TEST */
		TEST(munmap(array, memsize));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL, "munmap() failed: errno: %d: %s",
				 TEST_ERRNO, strerror(TEST_ERRNO));
		} else {
			tst_resm(TPASS, "munmap() was successful");
		}

		close(fd);
		unlink(filename);
	}
	cleanup();
	return 0;
}
