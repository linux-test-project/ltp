// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Linux Test Project
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) Renaud Lottiaux <Renaud.Lottiaux@kerlabs.com>
 * Ported to LTP: Wayne Boyer
 */

/*\
 * Test to check :man2:`execve` sets the following errnos correctly:
 *
 * 1. ENAMETOOLONG -- the file name is greater than VFS_MAXNAMELEN
 * 2. ENOENT -- the filename does not exist
 * 3. ENOTDIR -- the path contains a directory name which doesn't exist
 * 4. EFAULT -- the filename isn't part of the process address space
 * 5. EACCES -- the filename does not have execute permission
 * 6. ENOEXEC -- the file is zero length with execute permissions
 */

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>

#include "tst_test.h"

static char nobody_uid[] = "nobody";
static struct passwd *ltpuser;
static char long_fname[] = "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyzabcdefghijklmnopqrstmnopqrstuvwxyz";
static char no_dir[] = "testdir";
static char test_name3[1024];
static char test_name5[1024];
static char test_name6[1024];

static struct tcase {
	char *tname;
	int error;
} tcases[] = {
	/* the file name is greater than VFS_MAXNAMELEN - ENAMTOOLONG */
	{long_fname, ENAMETOOLONG},
	/* the filename does not exist - ENOENT */
	{no_dir, ENOENT},
	/* the path contains a directory name which doesn't exist - ENOTDIR */
	{test_name3, ENOTDIR},
	{NULL, EFAULT},
	{test_name5, EACCES},
	{test_name6, ENOEXEC}
};

static void setup(void)
{
	char *cwdname = NULL;
	unsigned i;
	int fd;

	umask(0);

	ltpuser = SAFE_GETPWNAM(nobody_uid);

	SAFE_SETGID(ltpuser->pw_gid);

	cwdname = SAFE_GETCWD(cwdname, 0);

	sprintf(test_name5, "%s/fake", cwdname);

	fd = SAFE_CREAT(test_name5, 0444);
	SAFE_CLOSE(fd);

	sprintf(test_name3, "%s/fake", test_name5);

	/* creat() and close a zero length file with executeable permission */
	sprintf(test_name6, "%s/execve03", cwdname);

	fd = SAFE_CREAT(test_name6, 0755);
	SAFE_CLOSE(fd);

	for (i = 0; i < ARRAY_SIZE(tcases); i++) {
		if (!tcases[i].tname)
			tcases[i].tname = tst_get_bad_addr(NULL);
	}
}

static void verify_execve(unsigned int i)
{
	struct tcase *tc = &tcases[i];
	char *argv[2] = {tc->tname, NULL};

	TEST(execve(tc->tname, argv, NULL));

	if (TST_RET != -1) {
		tst_res(TFAIL, "call succeeded unexpectedly");
		return;
	}

	if (TST_ERR == tc->error) {
		tst_res(TPASS | TTERRNO, "execve failed as expected");
		return;
	}

	tst_res(TFAIL | TTERRNO, "execve failed unexpectedly; expected %s",
		strerror(tc->error));
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_execve,
	.needs_root = 1,
	.needs_tmpdir = 1,
	.child_needs_reinit = 1,
	.setup = setup,
};
