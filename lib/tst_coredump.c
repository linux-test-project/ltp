// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2019 Red Hat, Inc.
 */

#define TST_NO_DEFAULT_MAIN

#include <sys/time.h>
#include <sys/resource.h>

#include "tst_test.h"
#include "tst_coredump.h"

void tst_no_corefile(int verbose)
{
	struct rlimit new_r, old_r;

	SAFE_GETRLIMIT(RLIMIT_CORE, &old_r);
	if (old_r.rlim_max >= 1 || geteuid() == 0) {
		/*
		 * 1 is a special value, that disables core-to-pipe.
		 * At the same time it is small enough value for
		 * core-to-file, so it skips creating cores as well.
		 */
		new_r.rlim_cur = 1;
		new_r.rlim_max = 1;
		SAFE_SETRLIMIT(RLIMIT_CORE, &new_r);

		if (verbose) {
			tst_res(TINFO,
				"Avoid dumping corefile for process(pid=%d)",
				getpid());
		}
	}
}
