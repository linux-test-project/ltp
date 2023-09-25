// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2023 SUSE LLC Marius Kittler <mkittler@suse.de>
 */

/*\
 * [Description]
 *
 * This is a regression test for hangup on pipe operations. See
 * https://www.spinics.net/lists/linux-api/msg49762.html for
 * additional context. It tests that pipe operations do not block
 * indefinitely when going to the soft limit on the total size of
 * all pipes created by a single user.
 */

#define _GNU_SOURCE
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "tst_test.h"
#include "tst_safe_stdio.h"
#include "tst_safe_macros.h"

static int pipe_count;
static int *pipes;
static char *buffer;

static void run(void)
{
	const int *const pipe = pipes + 2 * (pipe_count - 1);
	const int buffer_size = SAFE_FCNTL(pipe[1], F_GETPIPE_SZ);

	tst_res(TINFO, "Soft-limited buffer size: %d bytes", buffer_size);
	SAFE_WRITE(1, pipe[1], buffer, buffer_size);
	SAFE_READ(1, pipe[0], buffer, buffer_size - 1);
	SAFE_WRITE(1, pipe[1], buffer, 1);
	tst_res(TPASS, "Pipe operation did not block");

	SAFE_READ(1, pipe[0], buffer, 2);
}

static void setup(void)
{
	int pipe[2];
	int page_size = getpagesize(), soft_limit;
	struct rlimit nfd;

	SAFE_PIPE(pipe);
	const int buffer_size = SAFE_FCNTL(pipe[1], F_GETPIPE_SZ);
	SAFE_CLOSE(pipe[0]);
	SAFE_CLOSE(pipe[1]);

	SAFE_FILE_SCANF("/proc/sys/fs/pipe-user-pages-soft", "%i", &soft_limit);
	pipe_count = soft_limit * page_size / buffer_size;

	tst_res(TINFO, "Soft limit for pipes: %i pages", soft_limit);
	tst_res(TINFO, "Buffer size: %d byte", buffer_size);
	tst_res(TINFO, "Creating %i pipes", pipe_count);

	SAFE_GETRLIMIT(RLIMIT_NOFILE, &nfd);
	if (nfd.rlim_max < (unsigned long)pipe_count)
		tst_brk(TCONF, "NOFILE limit max too low: %lu < %i", nfd.rlim_max, pipe_count);
	if (nfd.rlim_cur < nfd.rlim_max) {
		nfd.rlim_cur = nfd.rlim_max;
		SAFE_SETRLIMIT(RLIMIT_NOFILE, &nfd);
	}

	buffer = SAFE_MALLOC(buffer_size);
	pipes = SAFE_MALLOC(pipe_count * 2 * sizeof(int));
	for (int i = 0; i < pipe_count; ++i)
		SAFE_PIPE(pipes + i * 2);

}

static void cleanup(void)
{
	for (int i = 0; i < pipe_count * 2; i++)
		if (pipes[i] > 0)
			SAFE_CLOSE(pipes[i]);
	if (pipes)
		free(pipes);
	if (buffer)
		free(buffer);
}

static struct tst_test test = {
	.setup = setup,
	.test_all = run,
	.cleanup = cleanup,
	.tags = (const struct tst_tag[]){
		{"linux-git", "46c4c9d1beb7"},
	},
};
