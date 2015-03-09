/*
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
 *
 * mmap001.c - Tests mmapping a big file and writing it once
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

char *TCID = "mmap001";
int TST_TOTAL = 5;
static char *filename = NULL;
static int m_opt = 0;
static char *m_copt;

static void cleanup(void)
{
	free(filename);

	tst_rmdir();
}

static void setup(void)
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

	if ((filename = strdup(buf)) == NULL) {
		tst_brkm(TBROK | TERRNO, cleanup, "strdup failed");
	}

}

static void help(void)
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
	char *array;
	int i, lc;
	int fd;
	unsigned int pages, memsize;

	tst_parse_opts(argc, argv, options, help);

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
		tst_count = 0;

		fd = open(filename, O_RDWR | O_CREAT, 0666);
		if ((fd == -1))
			tst_brkm(TBROK | TERRNO, cleanup,
				 "opening %s failed", filename);

		if (lseek(fd, memsize, SEEK_SET) != memsize) {
			TEST_ERRNO = errno;
			close(fd);
			tst_brkm(TBROK | TTERRNO, cleanup, "lseek failed");
		}

		if (write(fd, "\0", 1) != 1) {
			TEST_ERRNO = errno;
			close(fd);
			tst_brkm(TBROK | TTERRNO, cleanup,
				 "writing to %s failed", filename);
		}

		array = mmap(0, memsize, PROT_WRITE, MAP_SHARED, fd, 0);
		if (array == MAP_FAILED) {
			TEST_ERRNO = errno;
			close(fd);
			tst_brkm(TBROK | TTERRNO, cleanup,
				 "mmapping %s failed", filename);
		} else {
			tst_resm(TPASS, "mmap() completed successfully.");
		}

		tst_resm(TINFO, "touching mmaped memory");

		for (i = 0; i < memsize; i++) {
			array[i] = (char)i;
		}

		/*
		 * seems that if the map area was bad, we'd get SEGV,
		 * hence we can indicate a PASS.
		 */
		tst_resm(TPASS,
			 "we're still here, mmaped area must be good");

		TEST(msync(array, memsize, MS_SYNC));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO,
				 "synchronizing mmapped page failed");
		} else {
			tst_resm(TPASS,
				 "synchronizing mmapped page passed");
		}

		TEST(munmap(array, memsize));

		if (TEST_RETURN == -1) {
			tst_resm(TFAIL | TTERRNO,
				 "munmapping %s failed", filename);
		} else {
			tst_resm(TPASS, "munmapping %s successful", filename);
		}

		close(fd);
		unlink(filename);

	}
	cleanup();
	tst_exit();
}
