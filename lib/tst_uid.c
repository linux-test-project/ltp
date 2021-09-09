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

int tst_check_resuid_(const char *file, const int lineno, const char *callstr,
	uid_t exp_ruid, uid_t exp_euid, uid_t exp_suid)
{
	uid_t ruid, euid, suid;

	SAFE_GETRESUID(&ruid, &euid, &suid);

	if (ruid == exp_ruid && euid == exp_euid && suid == exp_suid)
		return 1;

	if (callstr) {
		tst_res_(file, lineno, TFAIL, "Unexpected process UID after %s",
			callstr);
	} else {
		tst_res_(file, lineno, TFAIL, "Unexpected process UID");
	}

	tst_res_(file, lineno, TINFO, "Got: ruid = %d, euid = %d, suid = %d",
		(int)ruid, (int)euid, (int)suid);
	tst_res_(file, lineno, TINFO,
		"Expected: ruid = %d, euid = %d, suid = %d",
		(int)exp_ruid, (int)exp_euid, (int)exp_suid);
	return 0;
}

int tst_check_resgid_(const char *file, const int lineno, const char *callstr,
	gid_t exp_rgid, gid_t exp_egid, gid_t exp_sgid)
{
	gid_t rgid, egid, sgid;

	SAFE_GETRESGID(&rgid, &egid, &sgid);

	if (rgid == exp_rgid && egid == exp_egid && sgid == exp_sgid)
		return 1;

	if (callstr) {
		tst_res_(file, lineno, TFAIL, "Unexpected process GID after %s",
			callstr);
	} else {
		tst_res_(file, lineno, TFAIL, "Unexpected process GID");
	}

	tst_res_(file, lineno, TINFO, "Got: rgid = %d, egid = %d, sgid = %d",
		(int)rgid, (int)egid, (int)sgid);
	tst_res_(file, lineno, TINFO,
		"Expected: rgid = %d, egid = %d, sgid = %d",
		(int)exp_rgid, (int)exp_egid, (int)exp_sgid);
	return 0;
}
