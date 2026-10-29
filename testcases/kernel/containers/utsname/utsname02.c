// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Clone two plain processes, change hostname in the first one then check if
 * hostaname has changed inside the second one as well.
 */

#define _GNU_SOURCE

#include "tst_test.h"

#define HOSTNAME "LTP_HOSTNAME"

static char *hostname1;
static char *hostname2;
static char originalhost[HOST_NAME_MAX];

static void reset_hostname(void)
{
	SAFE_SETHOSTNAME(originalhost, strlen(originalhost));
}

static void run(void)
{
	memset(hostname1, 0, HOST_NAME_MAX);
	memset(hostname2, 0, HOST_NAME_MAX);

	if (!SAFE_FORK()) {
		SAFE_SETHOSTNAME(HOSTNAME, strlen(HOSTNAME));
		SAFE_GETHOSTNAME(hostname1, HOST_NAME_MAX);

		TST_CHECKPOINT_WAKE(0);
		return;
	}

	if (!SAFE_FORK()) {
		TST_CHECKPOINT_WAIT(0);

		SAFE_GETHOSTNAME(hostname2, HOST_NAME_MAX);
		return;
	}

	tst_reap_children();

	TST_EXP_PASS(strcmp(hostname1, HOSTNAME));
	TST_EXP_PASS(strcmp(hostname2, HOSTNAME));

	reset_hostname();
}

static void setup(void)
{
	hostname1 = SAFE_MMAP(NULL, HOST_NAME_MAX, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	hostname2 = SAFE_MMAP(NULL, HOST_NAME_MAX, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	memset(originalhost, 0, HOST_NAME_MAX);

	SAFE_GETHOSTNAME(originalhost, HOST_NAME_MAX);
}

static void cleanup(void)
{
	SAFE_MUNMAP(hostname1, HOST_NAME_MAX);
	SAFE_MUNMAP(hostname2, HOST_NAME_MAX);

	reset_hostname();
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.needs_root = 1,
	.forks_child = 1,
	.needs_checkpoints = 1,
};
