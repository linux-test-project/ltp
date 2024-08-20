// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (c) International Business Machines Corp., 2001
 * Ported to LTP: Wayne Boyer
 */

/*\
 * [Description]
 *
 * Verify that, readlink(2) returns -1 and sets errno to
 *
 * 1. EACCES if search/write permission is denied in the directory where the
 * symbolic link esides.
 * 2. EINVAL if the buffer size is not positive.
 * 3. EINVAL if the specified file is not a symbolic link file.
 * 4. ENAMETOOLONG if the pathname component of symbolic link is too long
 * (ie, > PATH_MAX).
 * 5. ENOENT if the component of symbolic link points to an empty string.
 * 6. ENOTDIR if a component of the path prefix is not a directory.
 * 7. ELOOP if too many symbolic links were encountered in translating the
 * pathname.
 * 8. EFAULT if buf outside the process allocated address space.
 */

#include <pwd.h>
#include <errno.h>
#include <string.h>

#include "tst_test.h"

#define DIR_TEMP	"test_dir_1"
#define TEST_FILE1	"test_dir_1/test_file_1"
#define SYM_FILE1	"test_dir_1/slink_file_1"
#define TEST_FILE2	"test_file_2"
#define SYM_FILE2	"slink_file_2"
#define TEST_FILE3	"test_file_3"
#define SYM_FILE3	"test_file_3/slink_file_3"
#define ELOOPFILE	"/test_eloop"
#define TESTFILE	"test_file"
#define SYMFILE		"slink_file"

static char longpathname[PATH_MAX + 2];
static char elooppathname[sizeof(ELOOPFILE) * 43] = ".";
static char buffer[256];

static struct tcase {
	char *link;
	char *buf;
	size_t buf_size;
	int exp_errno;
} tcases[] = {
	{SYM_FILE1, buffer, sizeof(buffer), EACCES},
	{SYM_FILE2, buffer, 0, EINVAL},
	{TEST_FILE2, buffer, sizeof(buffer), EINVAL},
	{longpathname, buffer, sizeof(buffer), ENAMETOOLONG},
	{"", buffer, sizeof(buffer), ENOENT},
	{SYM_FILE3, buffer, sizeof(buffer), ENOTDIR},
	{elooppathname, buffer, sizeof(buffer), ELOOP},
	{SYMFILE, (char *)-1, sizeof(buffer), EFAULT},
};

static void verify_readlink(unsigned int n)
{
	struct tcase *tc = &tcases[n];

	TEST(readlink(tc->link, tc->buf, tc->buf_size));
	if (TST_RET != -1) {
		tst_res(TFAIL, "readlink() sueeeeded unexpectedly");
		return;
	}

	if (TST_ERR != tc->exp_errno) {
		tst_res(TFAIL | TTERRNO,
			"readlink() failed unexpectedly; expected: %d - %s, got",
			tc->exp_errno, tst_strerrno(tc->exp_errno));

		if (tc->exp_errno == ENOENT && TST_ERR == EINVAL) {
			tst_res(TWARN | TTERRNO,
				"It may be a Kernel Bug, see the patch:"
				"http://git.kernel.org/linus/1fa1e7f6");
		}
	} else {
		tst_res(TPASS | TTERRNO, "readlink() failed as expected");
	}
}

static void setup(void)
{
	int i;
	struct passwd *pwent;

	pwent = SAFE_GETPWNAM("nobody");
	SAFE_SETEUID(pwent->pw_uid);

	SAFE_MKDIR(DIR_TEMP, 0777);
	SAFE_TOUCH(TEST_FILE1, 0666, NULL);
	SAFE_SYMLINK(TEST_FILE1, SYM_FILE1);
	SAFE_CHMOD(DIR_TEMP, 0444);

	SAFE_TOUCH(TEST_FILE2, 0666, NULL);
	SAFE_SYMLINK(TEST_FILE2, SYM_FILE2);

	memset(longpathname, 'a', PATH_MAX + 1);

	SAFE_TOUCH(TEST_FILE3, 0666, NULL);

	SAFE_MKDIR("test_eloop", 0777);
	SAFE_SYMLINK("../test_eloop", "test_eloop/test_eloop");

	for (i = 0; i < 43; i++)
		strcat(elooppathname, ELOOPFILE);

	SAFE_TOUCH(TESTFILE, 0666, NULL);
	SAFE_SYMLINK(TESTFILE, SYMFILE);
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tcases),
	.test = verify_readlink,
	.setup = setup,
	.needs_tmpdir = 1,
	.needs_root = 1,
};
