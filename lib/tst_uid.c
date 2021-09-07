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

	for (ret = 1; ret < MAX_GID; ret++) {
		if (ret == skip || getgrgid(ret))
			continue;

		if (errno == 0 || errno == ENOENT || errno == ESRCH) {
			tst_res_(file, lineno, TINFO | TERRNO,
				"Found unused GID %d", (int)ret);
			return ret;
		}

		tst_brk_(file, lineno, TBROK|TERRNO, "Group ID lookup failed");
		return (gid_t)-1;
	}

	tst_brk_(file, lineno, TBROK, "No free group ID found");
	return (gid_t)-1;
}

void tst_get_uids(uid_t *buf, unsigned int start, unsigned int count)
{
	unsigned int i, j;
	uid_t id;

	for (i = start, id = 1; i < count; id++) {
		for (j = 0; j < start; j++) {
			if (buf[j] == id)
				break;
		}

		if (j >= start)
			buf[i++] = id;
	}
}

void tst_get_gids(gid_t *buf, unsigned int start, unsigned int count)
{
	unsigned int i, j;
	gid_t id;

	for (i = start, id = 1; i < count; id++) {
		for (j = 0; j < start; j++) {
			if (buf[j] == id)
				break;
		}

		if (j >= start)
			buf[i++] = id;
	}
}
