/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2024 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

#ifndef PTEM_H
#define PTEM_H

#define _GNU_SOURCE

#include "tst_test.h"

#define MASTERCLONE "/dev/ptmx"

static inline int open_master(void)
{
	int masterfd;

	if (access(MASTERCLONE, F_OK))
		tst_brk(TCONF, "%s device doesn't exist", MASTERCLONE);

	tst_res(TINFO, "opening master %s", MASTERCLONE);

	masterfd = SAFE_OPEN(MASTERCLONE, O_RDWR);

	if (grantpt(masterfd) == -1)
		tst_brk(TBROK | TERRNO, "grantpt() error");

	if (unlockpt(masterfd) == -1)
		tst_brk(TBROK | TERRNO, "unlockpt() error");

	return masterfd;
}

static inline int open_slave(const int masterfd)
{
	int slavefd;
	char *slavename;

	slavename = SAFE_PTSNAME(masterfd);

	tst_res(TINFO, "opening slave %s", slavename);

#ifndef __BIONIC__
	/* grantpt() is a no-op in bionic. */
	struct stat st;

	SAFE_STAT(slavename, &st);

	uid_t uid = getuid();

	if (st.st_uid != uid) {
		tst_brk(TBROK, "uid mismatch st.st_uid(%d) != getuid(%d)",
			st.st_uid, uid);
	}

	if (st.st_mode != (S_IFCHR | 0620)) {
		tst_brk(TBROK, "unexpected slave device permission: %o",
			st.st_mode);
	}
#endif

	slavefd = SAFE_OPEN(slavename, O_RDWR);

	return slavefd;
}

#endif
