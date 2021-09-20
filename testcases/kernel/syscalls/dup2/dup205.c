// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Ported from SPIE, section2/iosuite/dup6.c, by Airong Zhang
 */

/*\
 * [Description]
 *
 * Negative test for dup2() with max open file descriptors.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "tst_test.h"
#include "tst_safe_macros.h"

static int *fildes;
static int min;
static char pfilname[40];

static void setup(void)
{
	min = getdtablesize();	/* get number of files allowed open */
	fildes = SAFE_MALLOC((min + 10) * sizeof(int));
	memset(fildes, -1, (min + 10) * sizeof(int));
	sprintf(pfilname, "./dup205.%d\n", getpid());
}

static void cleanup(void)
{
	if (fildes != NULL)
		free(fildes);
}

static void run(void)
{
	int ifile = -1, rc = 0;

	fildes[0] = SAFE_CREAT(pfilname, 0666);
	fildes[fildes[0]] = fildes[0];
	for (ifile = fildes[0] + 1; ifile < min + 10; ifile++) {
		TEST(dup2(fildes[ifile - 1], ifile));
		fildes[ifile] = TST_RET;
		if (fildes[ifile] == -1)
			break;
		if (fildes[ifile] != ifile)
			tst_brk(TFAIL,
				"got wrong descriptor number back (%d != %d)",
				fildes[ifile], ifile);
	}

	if (ifile < min) {
		tst_res(TFAIL, "Not enough files duped");
		rc++;
	} else if (ifile > min) {
		tst_res(TFAIL, "Too many files duped");
		rc++;
	}
	if (TST_ERR != EBADF && TST_ERR != EMFILE && TST_ERR != EINVAL) {
		tst_res(TFAIL, "bad errno on dup2 failure");
		rc++;
	}

	if (rc)
		tst_res(TFAIL, "Test failed");
	else
		tst_res(TPASS, "Test passed");

	SAFE_UNLINK(pfilname);
	for (ifile = fildes[0]; ifile < min + 10; ifile++) {
		if (fildes[ifile] > 0)
			SAFE_CLOSE(fildes[ifile]);
	}
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
};
