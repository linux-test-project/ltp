// SPDX-License-Identifier: GPL-2.0-or-later

/*
 *  Copyright (c) International Business Machines  Corp., 2002
 */

/* 12/03/2002	Port to LTP     robbiew@us.ibm.com */
/* 06/30/2001	Port to Linux	nsharoff@us.ibm.com */

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mount.h>

#include "tst_test.h"

#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP| \
			 S_IXGRP|S_IROTH|S_IXOTH)
#define FILE_EISDIR		"."
#define FILE_EACCES		"/dev/null"
#define FILE_ENOENT		"/tmp/does/not/exist"
#define FILE_ENOTDIR		"./tmpfile/"
#define TEST_TMPFILE		"./tmpfile"
#define TEST_ELOOP		"test_file_eloop1"
#define TEST_ENAMETOOLONG	nametoolong
#define TEST_EROFS		"mntpoint/file"

static char nametoolong[PATH_MAX+2];
static struct passwd *ltpuser;

static void setup_euid(void)
{
	SAFE_SETEUID(ltpuser->pw_uid);
}

static void cleanup_euid(void)
{
	SAFE_SETEUID(0);
}

static struct test_case {
	char *filename;
	char *exp_errval;
	int exp_errno;
	void (*setupfunc) ();
	void (*cleanfunc) ();
} tcases[] = {
	{FILE_EISDIR, "EISDIR",  EISDIR,  NULL,   NULL},
	{FILE_EACCES, "EACCES",  EACCES,  NULL,   NULL},
	{FILE_ENOENT, "ENOENT",  ENOENT,  NULL,   NULL},
	{FILE_ENOTDIR, "ENOTDIR", ENOTDIR, NULL,   NULL},
	{TEST_TMPFILE, "EPERM",   EPERM,   setup_euid, cleanup_euid},
	{NULL,       "EPERM",   EPERM,   setup_euid, cleanup_euid},
	{TEST_ELOOP, "ELOOP",        ELOOP,        NULL, NULL},
	{TEST_ENAMETOOLONG, "ENAMETOOLONG", ENAMETOOLONG, NULL, NULL},
	{TEST_EROFS, "EROFS",        EROFS,        NULL, NULL},
};

static void setup(void)
{
	int fd;

	TEST(acct(NULL));
	if (TST_RET == -1 && TST_ERR == ENOSYS)
		tst_brk(TCONF, "acct() system call isn't configured in kernel");

	ltpuser = SAFE_GETPWNAM("nobody");

	fd = SAFE_CREAT(TEST_TMPFILE, 0777);
	SAFE_CLOSE(fd);

	TEST(acct(TEST_TMPFILE));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "acct failed unexpectedly");

	/* turn off acct, so we are in a known state */
	TEST(acct(NULL));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "acct(NULL) failed");

	/* ELOOP SETTING */
	SAFE_SYMLINK(TEST_ELOOP, "test_file_eloop2");
	SAFE_SYMLINK("test_file_eloop2", TEST_ELOOP);

	/* ENAMETOOLONG SETTING */
	memset(nametoolong, 'a', PATH_MAX+1);
}

static void verify_acct(unsigned int nr)
{
	struct test_case *tcase = &tcases[nr];

	if (tcase->setupfunc)
		tcase->setupfunc();

	TEST(acct(tcase->filename));

	if (tcase->cleanfunc)
		tcase->cleanfunc();

	if (TST_RET != -1) {
		tst_res(TFAIL, "acct(%s) succeeded unexpectedly",
				tcase->filename);
		return;
	}

	if (TST_ERR == tcase->exp_errno) {
		tst_res(TPASS | TTERRNO, "acct() failed as expected");
	} else {
		tst_res(TFAIL | TTERRNO,
				"acct() failed, expected: %s",
				tst_strerrno(tcase->exp_errno));
	}
}

static struct tst_test test = {
	.needs_root = 1,
	.mntpoint = "mntpoint",
	.needs_rofs = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.test = verify_acct,
};
