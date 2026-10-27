/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (c) 2021 Linux Test Project
 */

#ifndef TST_UID_H__
#define TST_UID_H__

#include <sys/types.h>

uid_t tst_get_free_uid_(const char *file, const int lineno, uid_t skip);

/**
 * tst_get_free_uid() - Find a UID not assigned to any user.
 * @skip: UID value to skip (pass 0 to skip none).
 *
 * Scans the password database for the first unused UID starting
 * from 1, skipping @skip. Calls tst_brk(TBROK) if no free UID
 * is found or a lookup error occurs.
 *
 * Return: An unused uid_t value.
 */
#define tst_get_free_uid(skip) tst_get_free_uid_(__FILE__, __LINE__, (skip))

/*
 * Find unassigned gid. The skip argument can be used to ignore e.g. the main
 * group of a specific user in case it's not listed in the group file. If you
 * do not need to skip any specific gid, simply set it to 0.
 */
gid_t tst_get_free_gid_(const char *file, const int lineno, gid_t skip);
#define tst_get_free_gid(skip) tst_get_free_gid_(__FILE__, __LINE__, (skip))

/*
 * Get a specific number of unique existing non-root user or group IDs.
 * The "start" parameter is the number of buffer entries that are already
 * filled and will not be modified. The function will fill the remaining
 * (size-start) entries with unique UID/GID values.
 */
void tst_get_uids(uid_t *buf, unsigned int start, unsigned int size);
void tst_get_gids(gid_t *buf, unsigned int start, unsigned int size);

/*
 * Helper functions for checking current proces UIDs/GIDs.
 */
int tst_check_resuid_(const char *file, const int lineno, const char *callstr,
	uid_t exp_ruid, uid_t exp_euid, uid_t exp_suid);
#define tst_check_resuid(cstr, ruid, euid, suid) \
	tst_check_resuid_(__FILE__, __LINE__, (cstr), (ruid), (euid), (suid))

int tst_check_resgid_(const char *file, const int lineno, const char *callstr,
	gid_t exp_rgid, gid_t exp_egid, gid_t exp_sgid);
#define tst_check_resgid(cstr, rgid, egid, sgid) \
	tst_check_resgid_(__FILE__, __LINE__, (cstr), (rgid), (egid), (sgid))

#endif /* TST_UID_H__ */
