// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Linux Test Project, 2010-2020
 * Copyright (c) International Business Machines Corp., 2009
 *
 * Authors:
 * Mimi Zohar <zohar@us.ibm.com>
 */

#include "tst_test.h"

#define MMAPSIZE 1024

static char *filename;
static void *file;
static int fd;

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

	tst_reinit();
	TST_CHECKPOINT_WAIT(0);
	/* keep running until ima_violations.sh open and close file */
	TST_CHECKPOINT_WAKE_AND_WAIT(0);

	tst_res(TPASS, "test completed");
}

static struct tst_test test = {
	.options = (struct tst_option[]) {
		{"f:", &filename, "File to mmap"},
		{}
	},
	.test_all = run,
	.cleanup = cleanup,
};
