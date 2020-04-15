// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2010-2020
 * Copyright (c) International Business Machines Corp., 2009
 *
 * Authors:
 * Mimi Zohar <zohar@us.ibm.com>
 */

#include "tst_test.h"

#define SLEEP_AFTER_CLOSE 3
#define MMAPSIZE 1024

static char *filename;
static void *file;
static int fd;

static struct tst_option options[] = {
	{"f:", &filename,
	 "-f file  File to mmap"},
	{NULL, NULL, NULL}
};

static void cleanup(void)
{
	if (file)
		SAFE_MUNMAP(file, MMAPSIZE);

	if (fd > 0)
		SAFE_CLOSE(fd);
}

static void run(void)
{
	if (!filename)
		tst_brk(TBROK, "missing filename (-f filename)");

	fd = SAFE_OPEN(filename, O_CREAT | O_RDWR, S_IRWXU);

	file = SAFE_MMAP(NULL, MMAPSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	SAFE_CLOSE(fd);

	tst_res(TINFO, "sleep %ds", SLEEP_AFTER_CLOSE);
	sleep(SLEEP_AFTER_CLOSE);

	tst_res(TPASS, "test completed");
}

static struct tst_test test = {
	.options = options,
	.test_all = run,
	.cleanup = cleanup,
};
