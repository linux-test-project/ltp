// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) Huawei Technologies Co., Ltd., 2015
 * Copyright (C) 2022 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Clone a process with CLONE_NEWPID flag and check for the maxium amount of
 * nested containers.
 */

#define _GNU_SOURCE

#include <sys/mman.h>
#include "tst_test.h"
#include "lapi/namespaces_constants.h"

#define MAXNEST 32

static int *level;

static int child_func(LTP_ATTRIBUTE_UNUSED void *arg)
{
	pid_t cpid;
	int status;

	if (*level == MAXNEST)
		return 0;

	(*level)++;

	cpid = ltp_clone_quick(CLONE_NEWPID | SIGCHLD, child_func, 0);
	if (cpid < 0)
		tst_brk(TBROK | TERRNO, "clone failed");

	SAFE_WAITPID(cpid, &status, 0);

	return 0;
}

static void setup(void)
{
	level = SAFE_MMAP(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void run(void)
{
	int ret, status;

	*level = 1;

	ret = ltp_clone_quick(CLONE_NEWPID | SIGCHLD, child_func, 0);
	if (ret < 0)
		tst_brk(TBROK | TERRNO, "clone failed");

	SAFE_WAITPID(ret, &status, 0);

	if (*level < MAXNEST) {
		tst_res(TFAIL, "Nested containers should be %d, but they are %d", MAXNEST, *level);
		return;
	}

	tst_res(TPASS, "All %d containers have been nested", MAXNEST);
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.setup = setup,
};
