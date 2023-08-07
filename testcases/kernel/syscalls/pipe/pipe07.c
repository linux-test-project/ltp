// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2002
 *  Ported by Paul Larson
 * Copyright (c) 2013 Cyril Hrubis <chrubis@suse.cz>
 * Copyright (c) 2023 SUSE LLC Avinesh Kumar <avinesh.kumar@suse.com>
 */

/*\
 * [Description]
 *
 * Verify that, pipe(2) syscall can open the maximum number of
 * file descriptors permitted.
 */

#include "tst_test.h"
#include <stdlib.h>

static int *opened_fds, *pipe_fds;
static int num_pipe_fds, exp_num_pipes;

static int record_open_fds(void)
{
	DIR *dir;
	struct dirent *ent;
	int fd;
	int num_opened_fds = 0;
	int arr_size = 0;

	dir = SAFE_OPENDIR("/proc/self/fd");

	while ((ent = SAFE_READDIR(dir))) {
		if (!strcmp(ent->d_name, ".") ||
			!strcmp(ent->d_name, ".."))
			continue;
		fd = atoi(ent->d_name);

		if (fd == dirfd(dir))
			continue;

		if (num_opened_fds >= arr_size) {
			arr_size = MAX(1, arr_size * 2);
			opened_fds = SAFE_REALLOC(opened_fds, arr_size * sizeof(int));
		}
		opened_fds[num_opened_fds++] = fd;
	}

	return num_opened_fds;
}

static void setup(void)
{
	int max_fds;

	max_fds = getdtablesize();
	tst_res(TINFO, "getdtablesize() = %d", max_fds);
	pipe_fds = SAFE_MALLOC(max_fds * sizeof(int));

	exp_num_pipes = (max_fds - record_open_fds()) / 2;
	tst_res(TINFO, "expected max fds to be opened by pipe(): %d", exp_num_pipes * 2);
}

static void run(void)
{
	int fds[2];

	do {
		TEST(pipe(fds));
		if (!TST_RET) {
			pipe_fds[num_pipe_fds++] = fds[0];
			pipe_fds[num_pipe_fds++] = fds[1];
		}
	} while (!TST_RET);

	TST_EXP_EQ_LI(errno, EMFILE);
	TST_EXP_EQ_LI(exp_num_pipes * 2, num_pipe_fds);

	for (int i = 0; i < num_pipe_fds; i++)
		SAFE_CLOSE(pipe_fds[i]);

	num_pipe_fds = 0;
}

static void cleanup(void)
{
	for (int i = 0; i < num_pipe_fds; i++)
		if (pipe_fds[i] > 0)
			SAFE_CLOSE(pipe_fds[i]);

	if (pipe_fds)
		free(pipe_fds);

	if (opened_fds)
		free(opened_fds);
}

static struct tst_test test = {
	.setup = setup,
	.cleanup = cleanup,
	.test_all = run
};
