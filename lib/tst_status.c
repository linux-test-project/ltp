// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2017 Cyril Hrubis <chrubis@suse.cz>
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"

static char buf[32];

const char *exited(int status)
{
	snprintf(buf, sizeof(buf), "exited with %i", WEXITSTATUS(status));

	return buf;
}

const char *signaled(int status)
{
	snprintf(buf, sizeof(buf), "killed by %s",
		tst_strsig(WTERMSIG(status)));

	return buf;
}

const char *invalid(int status)
{
	snprintf(buf, sizeof(buf), "invalid status 0x%x", status);

	return buf;
}

const char *tst_strstatus(int status)
{
	if (WIFEXITED(status))
		return exited(status);

	if (WIFSIGNALED(status))
		return signaled(status);

	if (WIFSTOPPED(status))
		return "is stopped";

	if (WIFCONTINUED(status))
		return "is resumed";

	return invalid(status);
}

int tst_validate_children_(const char *file, const int lineno,
	unsigned int count)
{
	unsigned int i;
	int status;
	pid_t pid;

	for (i = 0; i < count; i++) {
		pid = SAFE_WAITPID(-1, &status, 0);

		if (!WIFEXITED(status) || WEXITSTATUS(status)) {
			tst_res_(file, lineno, TFAIL, "Child %d: %s", pid,
				tst_strstatus(status));
			return 1;
		}
	}

	return 0;
}
