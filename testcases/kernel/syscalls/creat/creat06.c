// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 */

/*
 * DESCRIPTION
 *	Testcase to check creat(2) sets the following errnos correctly:
 *	1.	EISDIR
 *	2.	ENAMETOOLONG
 *	3.	ENOENT
 *	4.	ENOTDIR
 *	5.	EFAULT
 *	6.	EACCES
 *	7.	ELOOP
 *	8.	EROFS
 *
 *
 * ALGORITHM
 *	1.	Attempt to creat(2) an existing directory, and test for
 *		EISDIR
 *	2.	Attempt to creat(2) a file whose name is more than
 *		VFS_MAXNAMLEN and test for ENAMETOOLONG.
 *	3.	Attempt to creat(2) a file inside a directory which doesn't
 *		exist, and test for ENOENT
 *	4.	Attempt to creat(2) a file, the pathname of which comprises
 *		a component which is a file, test for ENOTDIR.
 *	5.	Attempt to creat(2) a file with a bad address
 *		and test for EFAULT
 *	6.	Attempt to creat(2) a file in a directory with no
 *		execute permission and test for EACCES
 *	7.	Attempt to creat(2) a file which links the other file that
 *		links the former and test for ELOOP
 *	8.	Attempt to creat(2) a file in a Read-only file system
 *		and test for EROFS
 */

#include <errno.h>
#include <string.h>
#include <limits.h>
#include <pwd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>

#include "tst_test.h"

#define	TEST_FILE	"test_dir"
#define	NO_DIR		"testfile/testdir"
#define	NOT_DIR		"file1/testdir"
#define	TEST6_FILE	"dir6/file6"
#define	TEST7_FILE	"file7"
#define	TEST8_FILE	"mntpoint/tmp"

#define	MODE1		0444
#define	MODE2		0666

static void setup(void);
static void test6_setup(void);
static void test6_cleanup(void);
#if !defined(UCLINUX)
static void bad_addr_setup(int);
#endif

static struct passwd *ltpuser;
static char long_name[PATH_MAX+2];

static struct test_case_t {
	char *fname;
	int mode;
	int error;
	void (*setup)();
	void (*cleanup)(void);
} tcases[] = {
	{TEST_FILE, MODE1, EISDIR, NULL, NULL},
	{long_name, MODE1, ENAMETOOLONG, NULL, NULL},
	{NO_DIR, MODE1, ENOENT, NULL, NULL},
	{NOT_DIR, MODE1, ENOTDIR, NULL, NULL},
#if !defined(UCLINUX)
	{NULL, MODE1, EFAULT, bad_addr_setup, NULL},
#endif
	{TEST6_FILE, MODE1, EACCES, test6_setup, test6_cleanup},
	{TEST7_FILE, MODE1, ELOOP, NULL, NULL},
	{TEST8_FILE, MODE1, EROFS, NULL, NULL},
};

static void verify_creat(unsigned int i)
{
	if (tcases[i].setup != NULL)
		tcases[i].setup(i);

	TEST(creat(tcases[i].fname, tcases[i].mode));

	if (tcases[i].cleanup != NULL)
		tcases[i].cleanup();

	if (TST_RET != -1) {
		tst_res(TFAIL, "call succeeded unexpectedly");
		return;
	}

	if (TST_ERR == tcases[i].error) {
		tst_res(TPASS | TTERRNO, "got expected failure");
		return;
	}

	tst_res(TFAIL | TTERRNO, "expected %s",
	         tst_strerrno(tcases[i].error));
}


static void setup(void)
{
	ltpuser = SAFE_GETPWNAM("nobody");

	SAFE_MKDIR(TEST_FILE, MODE2);

	memset(long_name, 'a', PATH_MAX+1);

	SAFE_TOUCH("file1", MODE1, NULL);

	SAFE_MKDIR("dir6", MODE2);

	SAFE_SYMLINK(TEST7_FILE, "test_file_eloop2");
	SAFE_SYMLINK("test_file_eloop2", TEST7_FILE);
}

#if !defined(UCLINUX)
static void bad_addr_setup(int i)
{
	if (tcases[i].fname)
		return;

	tcases[i].fname = SAFE_MMAP(0, 1, PROT_NONE,
	                            MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}
#endif

static void test6_setup(void)
{
	SAFE_SETEUID(ltpuser->pw_uid);
}

static void test6_cleanup(void)
{
	SAFE_SETEUID(0);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_creat,
	.needs_root = 1,
	.needs_rofs = 1,
	.mntpoint = "mntpoint",
	.setup = setup,
};
