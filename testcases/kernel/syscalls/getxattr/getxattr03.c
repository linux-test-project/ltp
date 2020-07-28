/*
 * Copyright (C) 2012 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it
 * is free of the rightful claim of any third person regarding
 * infringement or the like.  Any license provided herein, whether
 * implied or otherwise, applies only to this software file.  Patent
 * licenses, if any, provided herein do not apply to combinations of
 * this program with other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

/*
 * An empty buffer of size zero can be passed into getxattr(2) to return
 * the current size of the named extended attribute.
 */

/* Currently xattr is not enabled while mounting root file system. Patch is
 * to mount root file system with xattr enabled and then use it for the test.
*/

#include "config.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif
#include "test.h"
#include "safe_macros.h"

char *TCID = "getxattr03";

#ifdef HAVE_SYS_XATTR_H
#define XATTR_TEST_KEY "user.testkey"
#define XATTR_TEST_VALUE "test value"
#define XATTR_TEST_VALUE_SIZE (sizeof(XATTR_TEST_VALUE) - 1)
#define TESTFILE "mntpoint/getxattr03testfile"
#define DIR_MODE        (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define FILE_MODE       (S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID)
#define MNTPOINT        "mntpoint"

static const char *device = "/dev/vda";
static const char *fs_type = "ext4";
static void setup(void);
static void cleanup(void);

int TST_TOTAL = 1;

int main(int argc, char *argv[])
{
	int lc;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		tst_count = 0;

		TEST(getxattr(TESTFILE, XATTR_TEST_KEY, NULL, 0));

		if (TEST_RETURN == XATTR_TEST_VALUE_SIZE)
			tst_resm(TPASS, "getxattr(2) returned correct value");
		else
			tst_resm(TFAIL | TTERRNO, "getxattr(2) failed");
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int fd;

	tst_require_root();

	tst_tmpdir();
	rmdir(MNTPOINT);
	SAFE_MKDIR(cleanup, MNTPOINT, DIR_MODE);
	SAFE_MOUNT(cleanup, device, MNTPOINT, fs_type, 0, "user_xattr");

	/* Test for xattr support and set attr value */
	fd = SAFE_CREAT(cleanup, TESTFILE, 0644);
	SAFE_CLOSE(cleanup,fd);

	if (setxattr(TESTFILE, XATTR_TEST_KEY, XATTR_TEST_VALUE,
		     XATTR_TEST_VALUE_SIZE, XATTR_CREATE) == -1) {
		if (errno == ENOTSUP)
			tst_brkm(TCONF, cleanup, "No xattr support in fs or "
				 "fs mounted without user_xattr option");
		else
			tst_brkm(TBROK | TERRNO, cleanup, "setxattr %s failed",
				 TESTFILE);
	}

	TEST_PAUSE;
}

static void cleanup(void)
{
	remove(TESTFILE);
	SAFE_UMOUNT(NULL,MNTPOINT);
	SAFE_RMDIR(NULL,MNTPOINT);
	tst_rmdir();
}
#else /* HAVE_SYS_XATTR_H */
int main(int argc, char *argv[])
{
	tst_brkm(TCONF, NULL, "<sys/xattr.h> does not exist.");
}
#endif
