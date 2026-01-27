// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines  Corp., 2001
 * Copyright (C) 2026 SUSE LLC Andrea Cervesato <andrea.cervesato@suse.com>
 */

/*\
 * Fork a process using `vfork()` and verify that the attribute values like
 * euid, ruid, suid, egid, rgid, sgid, umask, inode and device number of
 * root and current working directories are the same of the parent
 * process ones.
 */

#include "tst_test.h"
#include "tst_uid.h"

static void run(void)
{
	char *p_cwd, *c_cwd;
	mode_t p_mask, c_mask;
	static uid_t p_ruid, p_euid, p_suid;
	static gid_t p_rgid, p_egid, p_sgid;
	struct stat p_cwd_stat, c_cwd_stat;
	struct stat p_root_stat, c_root_stat;

	p_mask = umask(0);
	umask(p_mask);

	p_cwd = getcwd(NULL, BUFSIZ);

	SAFE_GETRESUID(&p_ruid, &p_euid, &p_suid);
	SAFE_GETRESGID(&p_rgid, &p_egid, &p_sgid);

	SAFE_STAT(p_cwd, &p_cwd_stat);
	SAFE_STAT("/", &p_root_stat);

	if (!vfork()) {
		c_mask = umask(0);
		umask(c_mask);

		TST_EXP_EQ_LI(p_mask, c_mask);

		c_cwd = getcwd(NULL, BUFSIZ);
		SAFE_STAT(c_cwd, &c_cwd_stat);

		TST_EXP_EQ_STR(p_cwd, c_cwd);
		free(c_cwd);

		TST_EXP_EQ_LI(p_cwd_stat.st_ino, c_cwd_stat.st_ino);
		TST_EXP_EQ_LI(p_cwd_stat.st_dev, c_cwd_stat.st_dev);

		SAFE_STAT("/", &c_root_stat);

		TST_EXP_EQ_LI(p_root_stat.st_ino, c_root_stat.st_ino);
		TST_EXP_EQ_LI(p_root_stat.st_dev, c_root_stat.st_dev);

		if (tst_check_resuid("resuid()", p_ruid, p_euid, p_suid))
			tst_res(TPASS, "Parent and child UID are matching");

		if (tst_check_resgid("resgid()", p_rgid, p_egid, p_sgid))
			tst_res(TPASS, "Parent and child GID are matching");

		_exit(0);
	}

	tst_reap_children();

	free(p_cwd);
}

static struct tst_test test = {
	.test_all = run,
	.forks_child = 1,
};
