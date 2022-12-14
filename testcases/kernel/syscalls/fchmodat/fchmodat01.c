// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2006
 *
 * 08/28/2006 AUTHOR: Yi Yang <yyangcdl@cn.ibm.com>
 */

/*\
 * [Description]
 *
 * This test case will verify basic function of fchmodat.
 */

#define _GNU_SOURCE

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "tst_test.h"
#include "lapi/syscalls.h"

#ifndef AT_FDCWD
#define AT_FDCWD -100
#endif

static char pathname[256];
static char testfile[256];
static char testfile2[256];
static char testfile3[256];

static struct tcase {
	int exp_errno;
	char *exp_errval;
} tcases[] = {
	{ 0, NULL},
	{ 0, NULL},
	{ ENOTDIR, "ENOTDIR"},
	{ EBADF, "EBADF"},
	{ 0, NULL},
	{ 0, NULL},
};
static int fds[ARRAY_SIZE(tcases)];
static char *filenames[ARRAY_SIZE(tcases)];

static void verify_fchmodat(unsigned int i)
{
	struct tcase *tc = &tcases[i];

	if (tc->exp_errno == 0)
		TST_EXP_PASS(tst_syscall(__NR_fchmodat, fds[i], filenames[i], 0600),
			     "fchmodat() returned the expected errno %d: %s",
			     TST_ERR, strerror(TST_ERR));
	else
		TST_EXP_FAIL(tst_syscall(__NR_fchmodat, fds[i], filenames[i], 0600),
			     tc->exp_errno,
			     "fchmodat() returned the expected errno %d: %s",
			     TST_ERR, strerror(TST_ERR));
}

static void setup(void)
{
	/* Initialize test dir and file names */
	char *abs_path = tst_get_tmpdir();
	int p = getpid();

	sprintf(pathname, "fchmodattestdir%d", p);
	sprintf(testfile, "fchmodattest%d.txt", p);
	sprintf(testfile2, "%s/fchmodattest%d.txt", abs_path, p);
	sprintf(testfile3, "fchmodattestdir%d/fchmodattest%d.txt", p, p);

	free(abs_path);

	SAFE_MKDIR(pathname, 0700);

	fds[0] = SAFE_OPEN(pathname, O_DIRECTORY);
	fds[1] = fds[4] = fds[0];

	SAFE_FILE_PRINTF(testfile, "%s", testfile);
	SAFE_FILE_PRINTF(testfile2, "%s", testfile2);

	fds[2] = SAFE_OPEN(testfile3, O_CREAT | O_RDWR, 0600);
	fds[3] = 100;
	fds[5] = AT_FDCWD;

	filenames[0] = filenames[2] = filenames[3] = filenames[4] = testfile;
	filenames[1] = testfile2;
	filenames[5] = testfile3;
}

static void cleanup(void)
{
	if (fds[0] > 0)
		close(fds[0]);
	if (fds[2] > 0)
		close(fds[2]);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_fchmodat,
	.setup = setup,
	.cleanup = cleanup,
	.needs_tmpdir = 1,
};
