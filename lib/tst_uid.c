// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2021 Linux Test Project
 */

#include <sys/types.h>
#include <grp.h>
#include <errno.h>

#define TST_NO_DEFAULT_MAIN
#include "tst_test.h"
#include "tst_uid.h"

#define MAX_GID 32767

gid_t tst_get_free_gid_(const char *file, const int lineno, gid_t skip)
{
	gid_t ret;

	errno = 0;

	for (ret = 0; ret < MAX_GID; ret++) {
		if (ret == skip || getgrgid(ret))
			continue;

		if (errno == 0 || errno == ENOENT || errno == ESRCH)
			return ret;

		tst_brk_(file, lineno, TBROK|TERRNO, "Group ID lookup failed");
		return (gid_t)-1;
	}

	tst_brk_(file, lineno, TBROK, "No free group ID found");
	return (gid_t)-1;
}
