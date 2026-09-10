// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Linux Test Project
 */

#ifndef COREDUMP_COMMON_H
#define COREDUMP_COMMON_H

#include <elf.h>
#include <sys/wait.h>

#include "tst_test.h"
#include "lapi/prctl.h"

static char cwd[PATH_MAX];

static void set_pattern(const char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	SAFE_FILE_VPRINTF(PATH_KERN_CORE_PATTERN, fmt, va);
	va_end(va);
}

static pid_t crash_child(void)
{
	int status;
	pid_t pid;

	pid = SAFE_FORK();
	if (!pid)
		abort();

	SAFE_WAITPID(pid, &status, 0);

	if (!WIFSIGNALED(status) || !WCOREDUMP(status))
		tst_brk(TFAIL, "Child did not dump core");

	return pid;
}

static void coredump_setup(void)
{
	struct rlimit rl = {RLIM_INFINITY, RLIM_INFINITY};

	SAFE_SETRLIMIT(RLIMIT_CORE, &rl);
	SAFE_PRCTL(PR_SET_DUMPABLE, 1, 0, 0, 0);
	SAFE_GETCWD(cwd, sizeof(cwd));
}

#endif /* COREDUMP_COMMON_H */
