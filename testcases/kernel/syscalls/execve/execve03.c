// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) 2018 Linux Test Project
 * Copyright (c) International Business Machines Corp., 2001
 * Copyright (c) Renaud Lottiaux <Renaud.Lottiaux@kerlabs.com>
 * Ported to LTP: Wayne Boyer
 */

/*
 *	Testcase to check execve sets the following errnos correctly:
 *	1.	ENAMETOOLONG
 *	2.	ENOENT
 *	3.	ENOTDIR
 *	4.	EFAULT
 *	5.	EACCES
 *	6.	ENOEXEC
 *
 * ALGORITHM
 *	1.	Attempt to execve(2) a file whose name is more than
 *		VFS_MAXNAMLEN fails with ENAMETOOLONG.
 *
 *	2.	Attempt to execve(2) a file which doesn't exist fails with
 *		ENOENT.
 *
 *	3.	Attempt to execve(2) a pathname (executabl) comprising of a
 *		directory, which doesn't exist fails with ENOTDIR.
 *
 *	4.	Attempt to execve(2) a filename not within the address space
 *		of the process fails with EFAULT.
 *
 *	5.	Attempt to execve(2) a filename that does not have executable
 *		permission - fails with EACCES.
 *
 *	6.	Attempt to execve(2) a zero length file with executable
 *		permissions - fails with ENOEXEC.
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
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
	/* the filename isn't part of the process address space - EFAULT */
	{NULL, EFAULT},
	/* the filename does not have execute permission - EACCES */
	{test_name5, EACCES},
	/* the file is zero length with execute permissions - ENOEXEC */
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
