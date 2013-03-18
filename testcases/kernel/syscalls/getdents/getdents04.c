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

char *TCID = "getdents04";
int TST_TOTAL = 1;

static int exp_enos[] = { ENOTDIR, 0 };

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

int main(int ac, char **av)
{
	int lc;
	char *msg;
	int rval, fd;
	struct linux_dirent64 dir64;
	struct linux_dirent dir;

	if ((msg = parse_opts(ac, av, options, &help)) != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		if ((fd = open("test", O_CREAT | O_RDWR, 0777)) == -1)
			tst_brkm(TBROK | TERRNO, cleanup,
				 "open of file failed");

		if (longsyscall)
			rval = getdents64(fd, &dir64, sizeof(dir64));
		else
			rval = getdents(fd, &dir, sizeof(dir));

		/*
		 * Calling with a non directory file descriptor should give
		 * an ENOTDIR error.
		 */
		if (rval < 0) {
			TEST_ERROR_LOG(errno);

			switch (errno) {
			case ENOTDIR:
				tst_resm(TPASS,
					 "getdents failed as expected with ENOTDIR");
			break;
			case ENOSYS:
				tst_resm(TCONF, "syscall not implemented");
			break;
			break;
			default:
				tst_resm(TFAIL | TERRNO,
					 "getdents failed unexpectedly");
			break;
			}
		} else {
			tst_resm(TFAIL, "getdents call succeeded unexpectedly");
		}

		if (close(fd) == -1)
			tst_brkm(TBROK, cleanup, "fd close failed");
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	TEST_EXP_ENOS(exp_enos);

	TEST_PAUSE;
}

static void cleanup(void)
{
	TEST_CLEANUP;

	tst_rmdir();
}
