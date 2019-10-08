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

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "test.h"
#include "safe_macros.h"
#include "getdents.h"

static void cleanup(void);
static void setup(void);

static void reset_flags(void);
static void check_flags(void);
static void set_flag(const char *name);

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

enum entry_type {
	ENTRY_DIR,
	ENTRY_FILE,
	ENTRY_SYMLINK,
};

struct testcase {
	const char *name;
	enum entry_type type;
	int create:1;
	int found:1;
};

struct testcase testcases[] = {
	{.name = ".",       .create = 0, .type = ENTRY_DIR},
	{.name = "..",      .create = 0, .type = ENTRY_DIR},
	{.name = "dir",     .create = 1, .type = ENTRY_DIR},
	{.name = "file",    .create = 1, .type = ENTRY_FILE},
	{.name = "symlink", .create = 1, .type = ENTRY_SYMLINK},
};

/*
 * Big enough for dirp entires + data, the current size returned
 * by kernel is 128 bytes.
 */
#define BUFSIZE 512

int main(int ac, char **av)
{
	int lc;
	int rval, fd;
	struct linux_dirent64 *dirp64;
	struct linux_dirent *dirp;
	void *buf;

	tst_parse_opts(ac, av, options, &help);

	/* The buffer is allocated to make sure it's suitably aligned */
	buf = malloc(BUFSIZE);

	if (buf == NULL)
		tst_brkm(TBROK, NULL, "malloc failed");

	dirp64 = buf;
	dirp = buf;

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

		reset_flags();

		do {
			size_t d_reclen;

			if (longsyscall) {
				d_reclen = dirp64->d_reclen;
				d_name = dirp64->d_name;
			} else {
				d_reclen = dirp->d_reclen;
				d_name = dirp->d_name;
			}

			set_flag(d_name);

			tst_resm(TINFO, "Found '%s'", d_name);
			
			rval -= d_reclen;
			
			if (longsyscall)
				dirp64 = (void*)dirp64 + d_reclen;
			else
				dirp = (void*)dirp + d_reclen;

		} while (rval > 0);

		SAFE_CLOSE(cleanup, fd);
	
		check_flags();
	}

	free(buf);

	cleanup();
	tst_exit();
}

static void reset_flags(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(testcases); i++)
		testcases[i].found = 0;
}

static void check_flags(void)
{
	int i, err = 0;

	for (i = 0; i < ARRAY_SIZE(testcases); i++) {
		if (!testcases[i].found) {
			tst_resm(TINFO, "Entry '%s' not found", testcases[i].name);
			err++;
		}
	}

	if (err)
		tst_resm(TFAIL, "Some entires not found");
	else
		tst_resm(TPASS, "All entires found");
}

static void set_flag(const char *name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(testcases); i++) {
		if (!strcmp(name, testcases[i].name)) {
			testcases[i].found = 1;
			return;
		}
	}

	tst_resm(TFAIL, "Unexpected entry '%s' found", name);
}

static void setup(void)
{
	int i;

	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_tmpdir();

	for (i = 0; i < ARRAY_SIZE(testcases); i++) {
		
		if (!testcases[i].create)
			continue;
	
		switch (testcases[i].type) {
		case ENTRY_DIR:
			SAFE_MKDIR(cleanup, testcases[i].name, 0777);
		break;
		case ENTRY_FILE:
			SAFE_FILE_PRINTF(cleanup, testcases[i].name, " ");
		break;
		case ENTRY_SYMLINK:
			SAFE_SYMLINK(cleanup, "nonexistent", testcases[i].name);
		break;
		}
	}

	TEST_PAUSE;
}

static void cleanup(void)
{
	tst_rmdir();
}
