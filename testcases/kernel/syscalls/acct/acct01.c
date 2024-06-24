// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Copyright (c) International Business Machines  Corp., 2002
 * Copyright (c) Linux Test Project, 2003-2023
 * 12/03/2002	Port to LTP     robbiew@us.ibm.com
 * 06/30/2001	Port to Linux	nsharoff@us.ibm.com
 */

/*\
 * [Description]
 *
 * Verify that acct() returns proper errno on failure.
 */

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
#define FILE_EACCESS		"/dev/null"
#define FILE_ENOENT		"/tmp/does/not/exist"
#define FILE_ENOTDIR		"./tmpfile/"
#define FILE_TMPFILE		"./tmpfile"
#define FILE_ELOOP		"test_file_eloop1"
#define FILE_EROFS		"ro_mntpoint/file"
#define FILE_EFAULT		"invalid/file/name"

static struct passwd *ltpuser;

static char *file_eisdir;
static char *file_eaccess;
static char *file_enoent;
static char *file_enotdir;
static char *file_tmpfile;
static char *file_eloop;
static char *file_enametoolong;
static char *file_erofs;
static char *file_null;
static char *file_efault;

static void setup_euid(void)
{
	SAFE_SETEUID(ltpuser->pw_uid);
}

static void cleanup_euid(void)
{
	SAFE_SETEUID(0);
}

static void setup_emem(void)
{
	file_efault = SAFE_MMAP(NULL, 1, PROT_NONE,
			MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
}
static void cleanup_emem(void)
{
	SAFE_MUNMAP(file_efault, 1);
}

static struct test_case {
	char **filename;
	char *desc;
	int exp_errno;
	void (*setupfunc) ();
	void (*cleanfunc) ();
} tcases[] = {
	{&file_eisdir,  FILE_EISDIR,  EISDIR,  NULL,   NULL},
	{&file_eaccess, FILE_EACCESS, EACCES,  NULL,   NULL},
	{&file_enoent,  FILE_ENOENT,  ENOENT,  NULL,   NULL},
	{&file_enotdir, FILE_ENOTDIR, ENOTDIR, NULL,   NULL},
	{&file_tmpfile, FILE_TMPFILE, EPERM,   setup_euid, cleanup_euid},
	{&file_null,    "NULL",       EPERM,   setup_euid, cleanup_euid},
	{&file_eloop,   FILE_ELOOP,   ELOOP,        NULL, NULL},
	{&file_enametoolong, "aaaa...", ENAMETOOLONG, NULL, NULL},
	{&file_erofs,   FILE_EROFS,   EROFS,        NULL, NULL},
	{&file_efault,	"Invalid address",  EFAULT,  setup_emem, cleanup_emem},
};

static void setup(void)
{
	int fd;

	TEST(acct(NULL));
	if (TST_RET == -1 && TST_ERR == ENOSYS)
		tst_brk(TCONF, "acct() system call isn't configured in kernel");

	ltpuser = SAFE_GETPWNAM("nobody");

	fd = SAFE_CREAT(FILE_TMPFILE, 0777);
	SAFE_CLOSE(fd);

	TEST(acct(FILE_TMPFILE));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "acct failed unexpectedly");

	/* turn off acct, so we are in a known state */
	TEST(acct(NULL));
	if (TST_RET == -1)
		tst_brk(TBROK | TTERRNO, "acct(NULL) failed");

	/* ELOOP SETTING */
	SAFE_SYMLINK(FILE_ELOOP, "test_file_eloop2");
	SAFE_SYMLINK("test_file_eloop2", FILE_ELOOP);

	memset(file_enametoolong, 'a', PATH_MAX+1);
	file_enametoolong[PATH_MAX+1] = 0;
}

static void verify_acct(unsigned int nr)
{
	struct test_case *tcase = &tcases[nr];

	if (tcase->setupfunc)
		tcase->setupfunc();

	TST_EXP_FAIL(acct(*tcase->filename), tcase->exp_errno,
	             "acct(%s)", tcase->desc);

	if (tcase->cleanfunc)
		tcase->cleanfunc();
}

static struct tst_test test = {
	.needs_root = 1,
	.mntpoint = "ro_mntpoint",
	.needs_rofs = 1,
	.tcnt = ARRAY_SIZE(tcases),
	.setup = setup,
	.test = verify_acct,
	.bufs = (struct tst_buffers []) {
		{&file_eisdir, .str = FILE_EISDIR},
		{&file_eaccess, .str = FILE_EACCESS},
		{&file_enoent, .str = FILE_ENOENT},
		{&file_enotdir, .str = FILE_ENOTDIR},
		{&file_tmpfile, .str = FILE_TMPFILE},
		{&file_eloop, .str = FILE_ELOOP},
		{&file_enametoolong, .size = PATH_MAX+2},
		{&file_erofs, .str = FILE_EROFS},
		{}
	}
};
