// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * This test spawns multiple processes using fork() and it checks if wait()
 * returns the right PID once they end up.
 */

#include <stdlib.h>
#include "tst_test.h"

static char *str_numforks;
static int numforks = 1000;

static void run(void)
{
	pid_t pid;
	int status;
	int counter = 0;

	tst_res(TINFO, "Forking %d processes", numforks);

	for (int i = 0; i < numforks; i++) {
		pid = SAFE_FORK();
		if (!pid)
			exit(0);

		if (SAFE_WAIT(&status) == pid)
			counter++;
	}

	TST_EXP_EXPR(numforks == counter,
		"%d processes ended successfully",
		counter);
}

static void setup(void)
{
	if (tst_parse_int(str_numforks, &numforks, 1, INT_MAX))
		tst_brk(TBROK, "wrong number of forks '%s'", str_numforks);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.forks_child = 1,
	.options = (struct tst_option[]) {
		{ "n:", &str_numforks, "Number of processes to spawn" },
		{},
	},
};
