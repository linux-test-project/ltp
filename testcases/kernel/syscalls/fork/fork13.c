// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2010  Red Hat, Inc.
 * Copyright (C) 2022 Cyril Hrubis <chrubis@suse.cz>
 */

/*\
 * A race in pid generation that causes pids to be reused immediately
 *
 * From the mainline commit
 * 5fdee8c4a5e1 ("pids: fix a race in pid generation that causes pids to be reused immediately")
 *
 * A program that repeatedly forks and waits is susceptible to having
 * the same pid repeated, especially when it competes with another
 * instance of the same program.  This is really bad for bash
 * implementation.  Furthermore, many shell scripts assume that pid
 * numbers will not be used for some length of time.
 *
 * [Race Description] ::
 *
 *    A                                   B
 *
 *    // pid == offset == n               // pid == offset == n + 1
 *    test_and_set_bit(offset, map->page)
 *                                        test_and_set_bit(offset, map->page);
 *                                        pid_ns->last_pid = pid;
 *    pid_ns->last_pid = pid;
 *                                        // pid == n + 1 is freed (wait())
 *
 *                                        // Next fork()...
 *                                        last = pid_ns->last_pid; // == n
 *                                        pid = last + 1;
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "tst_test.h"

#define PID_MAX 32768
#define PID_MAX_STR "32768"
#define RETURN 256
#define MAX_ITERATIONS 1000000

/* The distance mod PIDMAX between two pids, where the first pid is
   expected to be smaller than the second. */
static int pid_distance(pid_t first, pid_t second)
{
	return (second + PID_MAX - first) % PID_MAX;
}

static void check(void)
{
	pid_t prev_pid = 0;
	pid_t pid;
	int i, distance, reaped, status, retval;

	for (i = 0; i < MAX_ITERATIONS; i++) {
		retval = i % RETURN;

		pid = SAFE_FORK();
		if (!pid)
			exit(retval);

		if (prev_pid) {
			distance = pid_distance(prev_pid, pid);
			if (distance == 0) {
				tst_res(TFAIL,
					"Unexpected pid sequence: prev_pid=%i, pid=%i for iteration=%i",
					prev_pid, pid, i);
				return;
			}
		}

		prev_pid = pid;

		reaped = SAFE_WAITPID(pid, &status, 0);

		if (reaped != pid) {
			tst_res(TFAIL,
				"Wrong pid %i returned from waitpid() expected %i",
				reaped, pid);
			return;
		}

		if (WEXITSTATUS(status) != retval) {
			tst_res(TFAIL,
				"Wrong process exit value %i expected %i",
				WEXITSTATUS(status), retval);
			return;
		}

		if (!tst_remaining_runtime()) {
			tst_res(TINFO, "Runtime exhausted, exiting...");
			break;
		}
	}

	tst_res(TPASS, "%i pids forked, all passed", i);
}

static struct tst_test test = {
	.needs_root = 1,
	.forks_child = 1,
	.runtime = 600,
	.test_all = check,
	.save_restore = (const struct tst_path_val[]) {
		{"/proc/sys/kernel/pid_max", PID_MAX_STR, TST_SR_TBROK},
		{}
	},
	.tags = (const struct tst_tag[]) {
		{"linux-git", "5fdee8c4a5e1"},
		{}
	}
};
