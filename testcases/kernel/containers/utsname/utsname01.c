// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2023 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Clone two plain processes and check if both read the same hostname.
 */

#define _GNU_SOURCE

#include "tst_test.h"

static char *hostname1;
static char *hostname2;

static void run(void)
{
	memset(hostname1, 0, HOST_NAME_MAX);
	memset(hostname2, 0, HOST_NAME_MAX);

	if (!SAFE_FORK()) {
		SAFE_GETHOSTNAME(hostname1, HOST_NAME_MAX);
		return;
	}

	if (!SAFE_FORK()) {
		SAFE_GETHOSTNAME(hostname2, HOST_NAME_MAX);
		return;
	}

	tst_reap_children();

	TST_EXP_PASS(strcmp(hostname1, hostname2));
}

static void setup(void)
{
	hostname1 = SAFE_MMAP(NULL, HOST_NAME_MAX, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	hostname2 = SAFE_MMAP(NULL, HOST_NAME_MAX, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

static void cleanup(void)
{
	SAFE_MUNMAP(hostname1, HOST_NAME_MAX);
	SAFE_MUNMAP(hostname2, HOST_NAME_MAX);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
	.forks_child = 1,
};
