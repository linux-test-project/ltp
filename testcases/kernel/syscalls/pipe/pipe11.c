// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 * Copyright (c) 2018 Cyril Hrubis <chrubis@suse.cz>
 */

/*
 * Check if many children can read what is written to a pipe by the parent.
 *
 * ALGORITHM
 *   For a different nchilds number:
 *	1. Open a pipe and write nchilds * (PIPE_BUF/nchilds) bytes into it
 *	2. Fork nchilds children
 *	3. Each child reads PIPE_BUF/nchilds characters and checks that the
 *	   bytes read are correct
 */
#include <stdlib.h>
#include "tst_test.h"

static int fds[2];
static unsigned char buf[PIPE_BUF];
static size_t read_per_child;

void do_child(void)
{
	size_t nread;
	unsigned char rbuf[read_per_child];
	unsigned int i;

	SAFE_CLOSE(fds[1]);

	nread = SAFE_READ(0, fds[0], rbuf, sizeof(rbuf));

	if (nread != read_per_child) {
		tst_res(TFAIL, "Invalid read size child %i size %zu",
		        getpid(), nread);
		return;
	}

	for (i = 0; i < read_per_child; i++) {
		if (rbuf[i] != (i % 256)) {
			tst_res(TFAIL,
			        "Invalid byte read child %i byte %i have %i expected %i",
				getpid(), i, rbuf[i], i % 256);
			return;
		}
	}

	tst_res(TPASS, "Child %i read pipe buffer correctly", getpid());
}

static unsigned int childs[] = {
	1,
	2,
	3,
	4,
	10,
	50
};

static void run(unsigned int tcase)
{
	pid_t pid;
	unsigned int nchilds = childs[tcase];
	read_per_child = PIPE_BUF/nchilds;
	unsigned int i, j;

	tst_res(TINFO, "Reading %zu per each of %u children",
	        read_per_child, nchilds);

	for (i = 0; i < nchilds; i++) {
		for (j = 0; j < read_per_child; j++) {
			buf[i * read_per_child + j] = j % 256;
		}
	}

	SAFE_PIPE(fds);

	SAFE_WRITE(1, fds[1], buf, read_per_child * nchilds);

	for (i = 0; i < nchilds; i++) {
		pid = SAFE_FORK();

		if (!pid) {
			do_child();
			exit(0);
		}
	}

	tst_reap_children();

	SAFE_CLOSE(fds[0]);
	SAFE_CLOSE(fds[1]);
}

static struct tst_test test = {
	.forks_child = 1,
	.test = run,
	.tcnt = ARRAY_SIZE(childs),
};
