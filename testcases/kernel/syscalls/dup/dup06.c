// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Ported from SPIE, section2/iosuite/dup1.c, by Airong Zhang
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) Linux Test Project, 2003-2024
 */

/*\
 * [Description]
 *
 * Test for dup(2) syscall with max open file descriptors.
 */

#include <stdlib.h>
#include "tst_test.h"

static int *pfildes;
static int minfd, maxfd, freefds;
static char pfilname[40];

static int cnt_free_fds(int maxfd)
{
	int freefds = 0;

	for (maxfd--; maxfd >= 0; maxfd--)
		if (fcntl(maxfd, F_GETFD) == -1 && errno == EBADF)
			freefds++;

	return freefds;
}

static void setup(void)
{
	minfd = getdtablesize();	/* get number of files allowed open */
	maxfd = minfd + 5;
	freefds = cnt_free_fds(minfd);
	pfildes = SAFE_MALLOC(maxfd * sizeof(int));
	memset(pfildes, -1, maxfd * sizeof(int));
	sprintf(pfilname, "./dup06.%d\n", getpid());
}

static void cleanup(void)
{
	if (pfildes != NULL)
		free(pfildes);
}

static void run(void)
{
	int i;

	pfildes[0] = SAFE_CREAT(pfilname, 0666);
	for (i = 1; i < maxfd; i++) {
		pfildes[i] = dup(pfildes[i - 1]);
		if (pfildes[i] == -1)
			break;
	}
	if (i < freefds)
		tst_res(TFAIL, "Not enough files duped");
	else if (i > freefds)
		tst_res(TFAIL, "Too many files duped");
	else
		tst_res(TPASS, "Test passed");

	SAFE_UNLINK(pfilname);

	for (i = 0; i < maxfd; i++) {
		if (pfildes[i] != 0 && pfildes[i] != -1)
			SAFE_CLOSE(pfildes[i]);
	}
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
};
