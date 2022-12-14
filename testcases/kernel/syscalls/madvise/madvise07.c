// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2016 Richard Palethorpe <richiejp@f-m.fm>
 */

/*
 * Check that accessing a page marked with MADV_HWPOISON results in SIGBUS.
 *
 * Test flow: create child process,
 *	      map and write to memory,
 *	      mark memory with MADV_HWPOISON,
 *	      access memory,
 *	      if SIGBUS is delivered to child the test passes else it fails
 *
 * If the underlying page type of the memory we have mapped does not support
 * poisoning then the test will fail. We try to map and write to the memory in
 * such a way that by the time madvise is called the virtual memory address
 * points to a supported page. However there may be some rare circumstances
 * where the test produces the wrong result because we have somehow obtained
 * an unsupported page. In such cases madvise will probably return success,
 * but no SIGBUS will be produced.
 *
 * For more information see <linux source>/Documentation/vm/hwpoison.txt.
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "tst_test.h"
#include "lapi/mmap.h"

static void run_child(void)
{
	const size_t msize = getpagesize();
	void *mem = NULL;

	tst_res(TINFO,
		"mmap(0, %zu, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0)",
		msize);
	mem = SAFE_MMAP(NULL,
			msize,
			PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_PRIVATE,
			-1,
			0);
	memset(mem, 'L', msize);

	tst_res(TINFO, "madvise(%p, %zu, MADV_HWPOISON)", mem, msize);
	if (madvise(mem, msize, MADV_HWPOISON) == -1) {
		if (errno == EINVAL) {
			tst_res(TCONF | TERRNO,
				"CONFIG_MEMORY_FAILURE probably not set in kconfig");
		} else {
			tst_res(TFAIL | TERRNO, "Could not poison memory");
		}
		exit(0);
	}

	*((char *)mem) = 'd';

	tst_res(TFAIL, "Did not receive SIGBUS on accessing poisoned page");
}

static void run(void)
{
	int status;
	pid_t pid;

	pid = SAFE_FORK();
	if (pid == 0) {
		run_child();
		exit(0);
	}

	SAFE_WAITPID(pid, &status, 0);
	if (WIFSIGNALED(status) && WTERMSIG(status) == SIGBUS) {
		tst_res(TPASS, "Received SIGBUS after accessing poisoned page");
		return;
	}

	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return;

	tst_res(TFAIL, "Child %s", tst_strstatus(status));
}

static struct tst_test test = {
	.test_all = run,
	.needs_root = 1,
	.forks_child = 1
};

