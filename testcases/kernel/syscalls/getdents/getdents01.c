/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	         written by Wayne Boyer
 * Copyright (c) 2013 Markos Chandras
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

#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "test.h"
#include "usctest.h"
#include "getdents.h"

static void cleanup(void);
static void setup(void);

char *TCID = "getdents01";
int TST_TOTAL = 1;

static int longsyscall;

static option_t options[] = {
		/* -l long option. Tests getdents64 */
		{"l", &longsyscall, NULL},
		{NULL, NULL, NULL}
};

static void help(void)
{
	printf("  -l      Test the getdents64 system call\n");
}

/* Big enough for both dirp entires + data */
#define BUFSIZE 512

int main(int ac, char **av)
{
	int lc;
	char *msg;
	int rval, fd;
	struct linux_dirent64 *dirp64;
	struct linux_dirent *dirp;
	void *buf;

	/* The buffer is allocated to make sure it's suitably aligned */
	buf = malloc(BUFSIZE);

	if (buf == NULL)
		tst_brkm(TBROK, NULL, "malloc failed");

	dirp64 = buf;
	dirp = buf;

	if ((msg = parse_opts(ac, av, options, &help)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		const char *d_name;

		tst_count = 0;

		if ((fd = open(".", O_RDONLY)) == -1)
			tst_brkm(TBROK, cleanup, "open of directory failed");

		if (longsyscall)
			rval = getdents64(fd, dirp64, BUFSIZE);
		else
			rval = getdents(fd, dirp, BUFSIZE);

		if (rval < 0) {
			if (errno == ENOSYS)
				tst_resm(TCONF, "syscall not implemented");
			else
				tst_resm(TFAIL | TERRNO,
				         "getdents failed unexpectedly");
			continue;
		}

		if (rval == 0) {
			tst_resm(TFAIL,
				 "getdents failed - returned end of directory");
			continue;
		}

		if (longsyscall)
			d_name = dirp64->d_name;
		else
			d_name = dirp->d_name;

		if (strcmp(d_name, ".") && strcmp(d_name, ".."))
			tst_resm(TINFO, "First entry is not '.' or '..'");
		else
			tst_resm(TPASS, "call succeeded");

		if (close(fd) == -1)
			tst_brkm(TBROK, cleanup, "file close failed");
	}

	free(buf);

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	TEST_PAUSE;
}

static void cleanup(void)
{
	TEST_CLEANUP;

	tst_rmdir();
}
