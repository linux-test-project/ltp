/*
 * Copyright (c) 2013 Fujitsu Ltd.
 * Author: Xiaoguang Wang <wangxg.fnst@cn.fujitsu.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/*
 * Description:
 * Verify that,
 *  1. access() fails with -1 return value and sets errno to EROFS
 *     if write permission was requested for files on a read-only file system.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <pwd.h>

#include "test.h"
#include "usctest.h"
#include "safe_macros.h"


static void setup(void);
static void access_verify(int i);
static void cleanup(void);
static void help(void);

#define DIR_MODE	(S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP| \
			 S_IXGRP|S_IROTH|S_IXOTH)
#define MNT_POINT	"mntpoint"

static char *fstype = "ext2";
static char *device;
static int dflag;
static int mount_flag;

static option_t options[] = {
	{"T:", NULL, &fstype},
	{"D:", &dflag, &device},
	{NULL, NULL, NULL}
};

static struct test_case_t {
	char *pathname;
	int a_mode;
	int exp_errno;
} test_cases[] = {
	{MNT_POINT, W_OK, EROFS}
};

char *TCID = "access06";
int TST_TOTAL = ARRAY_SIZE(test_cases);
static int exp_enos[] = { EROFS, 0 };

int main(int ac, char **av)
{
	int lc;
	char *msg;
	int i;

	msg = parse_opts(ac, av, options, help);
	if (msg != NULL)
		tst_brkm(TBROK, NULL, "OPTION PARSING ERROR - %s", msg);

	/* Check for mandatory option of the testcase */
	if (!dflag) {
		tst_brkm(TBROK, NULL, "you must specify the device "
			 "used for mounting with -D option");
	}

	setup();

	TEST_EXP_ENOS(exp_enos);

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		for (i = 0; i < TST_TOTAL; i++)
			access_verify(i);
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	tst_sig(NOFORK, DEF_HANDLER, cleanup);

	tst_require_root(NULL);

	tst_mkfs(NULL, device, fstype, NULL);

	tst_tmpdir();

	SAFE_MKDIR(cleanup, MNT_POINT, DIR_MODE);

	TEST_PAUSE;

	/*
	 * mount a read-only file system for test EROFS
	 */
	if (mount(device, MNT_POINT, fstype, MS_RDONLY, NULL) < 0) {
		tst_brkm(TBROK | TERRNO, cleanup,
			 "mount device:%s failed", device);
	}
	mount_flag = 1;
}

static void access_verify(int i)
{
	char *file_name;
	int access_mode;

	file_name = test_cases[i].pathname;
	access_mode = test_cases[i].a_mode;

	TEST(access(file_name, access_mode));

	if (TEST_RETURN != -1) {
		tst_resm(TFAIL, "access(%s, %#o) succeeded unexpectedly",
			 file_name, access_mode);
		return;
	}

	if (TEST_ERRNO == test_cases[i].exp_errno) {
		tst_resm(TPASS | TTERRNO, "access failed as expected");
	} else {
		tst_resm(TFAIL | TTERRNO,
			 "access failed unexpectedly; expected: "
			 "%d - %s", test_cases[i].exp_errno,
			 strerror(test_cases[i].exp_errno));
	}
}

static void cleanup(void)
{
	TEST_CLEANUP;

	if (mount_flag && umount(MNT_POINT) < 0) {
		tst_brkm(TBROK | TERRNO, NULL,
			 "umount device:%s failed", device);
	}
	tst_rmdir();
}

static void help(void)
{
	printf("-T type	  : specifies the type of filesystem to be mounted. "
	       "Default ext2.\n");
	printf("-D device : device used for mounting.\n");
}
