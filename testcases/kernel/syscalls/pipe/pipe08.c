// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 *  07/2001 Ported by Wayne Boyer
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * Verify that, on any attempt to write to a pipe which is closed for
 * reading will generate a SIGPIPE signal and write will fail with
 * EPIPE errno.
 */

#include "tst_test.h"

static int pipefd[2];
static volatile int sigpipe_cnt;

static void sighandler(int sig)
{
	if (sig == SIGPIPE)
		sigpipe_cnt++;
}

static void run(void)
{
	char wrbuf[] = "abcdefghijklmnopqrstuvwxyz";

	sigpipe_cnt = 0;

	SAFE_PIPE(pipefd);
	SAFE_CLOSE(pipefd[0]);

	TST_EXP_FAIL2_SILENT(write(pipefd[1], wrbuf, sizeof(wrbuf)), EPIPE);
	TST_EXP_EQ_LI(sigpipe_cnt, 1);

	SAFE_CLOSE(pipefd[1]);
}

static void setup(void)
{
	SAFE_SIGNAL(SIGPIPE, sighandler);
}

static void cleanup(void)
{
	if (pipefd[0] > 0)
		SAFE_CLOSE(pipefd[0]);
	if (pipefd[1] > 0)
		SAFE_CLOSE(pipefd[1]);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.cleanup = cleanup
};
