/*
 * Copyright (c) International Business Machines  Corp., 2002
 *               Ported by Paul Larson
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 *
 * This program is free software;  you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY;  without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;  if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/*
 * Test the ability of pipe to open the maximum even number of file
 * descriptors permitted (or (maxfds - 3)/2 pipes)
 *
 * ALGORITHM
 *      1. record file descriptors open prior to test run
 * 	2. open pipes until EMFILE is returned
 * 	3. check to see that the number of pipes opened is (maxfds - 3) / 2
 * 	4. close all fds in range 0, maximal fd that were not open prior to
 * 	   the test execution
 */
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

#include "test.h"
#include "safe_macros.h"

char *TCID = "pipe07";
int TST_TOTAL = 1;

/* used to record file descriptors open at the test start */
static int rec_fds[128];
static int rec_fds_max;
static void record_open_fds(void);
static void close_test_fds(int max_fd);

static void setup(void);
static void cleanup(void);

int main(int ac, char **av)
{
	int lc;
	int min, ret;
	int npipes;
	int pipes[2], max_fd = 0;

	tst_parse_opts(ac, av, NULL, NULL);

	setup();

	min = getdtablesize() - rec_fds_max;

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (npipes = 0;; npipes++) {
			ret = pipe(pipes);
			if (ret < 0) {
				if (errno != EMFILE) {
					tst_brkm(TFAIL, cleanup,
						 "got unexpected error - %d",
						 errno);
				}
				break;
			}

			max_fd = MAX(pipes[0], max_fd);
			max_fd = MAX(pipes[1], max_fd);
		}

		if (npipes == (min / 2))
			tst_resm(TPASS, "Opened %d pipes", npipes);
		else
			tst_resm(TFAIL, "Unable to open maxfds/2 pipes");

		close_test_fds(max_fd);
		max_fd = 0;
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(FORK, DEF_HANDLER, cleanup);
	TEST_PAUSE;

	record_open_fds();
}

static void record_open_fds(void)
{
	DIR *dir = opendir("/proc/self/fd");
	int dir_fd, fd;
	struct dirent *file;

	if (dir == NULL)
		tst_brkm(TBROK | TERRNO, cleanup, "opendir()");

	dir_fd = dirfd(dir);

	if (dir_fd == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "dirfd()");

	errno = 0;

	while ((file = readdir(dir))) {
		if (!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))
			continue;

		fd = atoi(file->d_name);

		if (fd == dir_fd)
			continue;

		if (rec_fds_max >= (int)ARRAY_SIZE(rec_fds)) {
			tst_brkm(TBROK, cleanup,
			         "Too much file descriptors open");
		}

		rec_fds[rec_fds_max++] = fd;
	}

	if (errno)
		tst_brkm(TBROK | TERRNO, cleanup, "readdir()");

	closedir(dir);

	tst_resm(TINFO, "Found %u files open", rec_fds_max);
}

static int not_recorded(int fd)
{
	int i;

	for (i = 0; i < rec_fds_max; i++)
		if (fd == rec_fds[i])
			return 0;

	return 1;
}

static void close_test_fds(int max_fd)
{
	int i;

	for (i = 0; i <= max_fd; i++) {
		if (not_recorded(i)) {
			if (close(i)) {
				if (errno == EBADF)
					continue;
				tst_resm(TWARN | TERRNO, "close(%i)", i);
			}
		}
	}
}

static void cleanup(void)
{
}
