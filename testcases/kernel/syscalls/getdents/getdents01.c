// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *	         written by Wayne Boyer
 * Copyright (c) 2013 Markos Chandras
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 *
 * [DESCRIPTION]
 *
 * Basic getdents() test that checks if directory listing is correct and
 * complete.
 *
\*/

#define _GNU_SOURCE

#include "tst_test.h"
#include "getdents.h"

#include <stdlib.h>

static void reset_flags(void);
static void check_flags(void);
static void set_flag(const char *name);

static int fd;

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

static void *dirp;

static void run(void)
{
	int rval;

	rval = tst_getdents(fd, dirp, BUFSIZE);

	if (rval < 0) {
		if (errno == ENOSYS)
			tst_brk(TCONF, "syscall not implemented");
		else {
			tst_res(TFAIL | TERRNO, "getdents failed unexpectedly");
			return;
		}
	}

	if (rval == 0) {
		tst_res(TFAIL, "getdents failed - returned end of directory");
		return;
	}

	reset_flags();

	do {
		size_t d_reclen = tst_dirp_reclen(dirp);
		const char *d_name = tst_dirp_name(dirp);

		set_flag(d_name);

		tst_res(TINFO, "Found '%s'", d_name);

		rval -= d_reclen;
		dirp += d_reclen;
	} while (rval > 0);

	check_flags();
}

static void reset_flags(void)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(testcases); i++)
		testcases[i].found = 0;
}

static void check_flags(void)
{
	size_t i;
	int err = 0;

	for (i = 0; i < ARRAY_SIZE(testcases); i++) {
		if (!testcases[i].found) {
			tst_res(TINFO, "Entry '%s' not found", testcases[i].name);
			err++;
		}
	}

	if (err)
		tst_res(TFAIL, "Some entries not found");
	else
		tst_res(TPASS, "All entries found");
}

static void set_flag(const char *name)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(testcases); i++) {
		if (!strcmp(name, testcases[i].name)) {

			if (testcases[i].found)
				tst_res(TFAIL, "Duplicate entry %s", name);

			testcases[i].found = 1;
			return;
		}
	}

	tst_res(TFAIL, "Unexpected entry '%s' found", name);
}

static void setup(void)
{
	size_t i;

	getdents_info();

	if (!tst_variant) {
		for (i = 0; i < ARRAY_SIZE(testcases); i++) {
			if (!testcases[i].create)
				continue;

			switch (testcases[i].type) {
			case ENTRY_DIR:
				SAFE_MKDIR(testcases[i].name, 0777);
			break;
			case ENTRY_FILE:
				SAFE_FILE_PRINTF(testcases[i].name, " ");
			break;
			case ENTRY_SYMLINK:
				SAFE_SYMLINK("nonexistent", testcases[i].name);
			break;
			}
		}
	}

	fd = SAFE_OPEN(".", O_RDONLY|O_DIRECTORY);
}

static void cleanup(void)
{
	if (fd != 0)
		SAFE_CLOSE(fd);
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.bufs = (struct tst_buffers []) {
		{&dirp, .size = BUFSIZE},
		{},
	},
	.test_variants = TEST_VARIANTS,
};
