// SPDX-License-Identifier: GPL-2.0-or-later

/*
 *   Copyright (c) International Business Machines  Corp., 2002
 *   01/02/2003	Port to LTP	avenkat@us.ibm.com
 *   11/11/2002: Ported to LTP Suite by Ananda
 *   06/30/2001	Port to Linux	nsharoff@us.ibm.com
 */

/*\
 * Checks that process which called abort() gets killed by SIGIOT and dumps core.
 *
 * [Algorithm]
 *  - Fork child.
 *  - Child calls abort.
 *  - Parent checks return status.
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>

#include "tst_test.h"

static void do_child(void)
{
	abort();
	tst_res(TFAIL, "Abort returned");
	exit(0);
}

void verify_abort(void)
{
	int status, kidpid;
	int sig, core;

	kidpid = SAFE_FORK();
	if (kidpid == 0)
		do_child();

	SAFE_WAIT(&status);

	if (!WIFSIGNALED(status)) {
		tst_res(TFAIL, "Child %s, expected SIGIOT",
			tst_strstatus(status));
		return;
	}

	core = WCOREDUMP(status);
	sig = WTERMSIG(status);

	if (core == 0)
		tst_res(TFAIL, "abort() failed to dump core");
	else
		tst_res(TPASS, "abort() dumped core");

	if (sig == SIGIOT)
		tst_res(TPASS, "abort() raised SIGIOT");
	else
		tst_res(TFAIL, "abort() raised %s", tst_strsig(sig));
}

#define MIN_RLIMIT_CORE (512 * 1024)

static void setup(void)
{
	struct rlimit rlim;

	/* make sure we get core dumps */
	SAFE_GETRLIMIT(RLIMIT_CORE, &rlim);

	if (rlim.rlim_max < MIN_RLIMIT_CORE) {
		if (geteuid() != 0) {
			tst_brk(TCONF, "hard limit(%lu)less than MIN_RLIMT_CORE(%i)",
				rlim.rlim_max, MIN_RLIMIT_CORE);
		}
		tst_res(TINFO, "Raising rlim_max to %i", MIN_RLIMIT_CORE);
		rlim.rlim_max = MIN_RLIMIT_CORE;
	}
	if (rlim.rlim_cur < MIN_RLIMIT_CORE) {
		rlim.rlim_cur = MIN_RLIMIT_CORE;
		SAFE_SETRLIMIT(RLIMIT_CORE, &rlim);
	}
}

static struct tst_test test = {
	.needs_tmpdir = 1,
	.forks_child = 1,
	.setup = setup,
	.test_all = verify_abort,
};
