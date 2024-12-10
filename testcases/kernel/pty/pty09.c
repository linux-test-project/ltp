// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that slave pseudo-terminal can be opened multiple times in parallel.
 */

#define _GNU_SOURCE

#include "common.h"

static int masterfd = -1;

static unsigned int count_avail_pid(void)
{
	DIR *dir;
	struct dirent *ent;
	struct rlimit limit;
	unsigned int count = 0;
	unsigned int max_pid_num;

	SAFE_GETRLIMIT(RLIMIT_NOFILE, &limit);

	dir = SAFE_OPENDIR("/proc/self/fd");
	while ((ent = SAFE_READDIR(dir)))
		count++;

	SAFE_CLOSEDIR(dir);

	max_pid_num = limit.rlim_cur - count;

	tst_res(TINFO, "Available number of pids: %u", max_pid_num);

	return max_pid_num;
}

static void run(void)
{
	unsigned int max_pid_num;

	max_pid_num = count_avail_pid();

	int slavefd[max_pid_num];

	for (uint32_t i = 0; i < max_pid_num; i++)
		slavefd[i] = open_slave(masterfd);

	tst_res(TPASS, "pty has been opened %d times", max_pid_num);

	for (uint32_t i = 0; i < max_pid_num; i++)
		SAFE_CLOSE(slavefd[i]);
}

static void setup(void)
{
	masterfd = open_master();
}

static void cleanup(void)
{
	if (masterfd != -1)
		SAFE_CLOSE(masterfd);
}

static struct tst_test test = {
	.test_all = run,
	.setup = setup,
	.cleanup = cleanup,
};
