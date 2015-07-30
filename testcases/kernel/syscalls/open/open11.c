/*
 * Copyright (C) 2013 Red Hat, Inc.
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
 * Basic tests for open(2) and make sure open(2) works and handles error
 * conditions correctly.
 *
 * There are 28 test cases:
 * 1. Open regular file O_RDONLY
 * 2. Open regular file O_WRONLY
 * 3. Open regular file O_RDWR
 * 4. Open regular file O_RDWR | O_SYNC
 * 5. Open regular file O_RDWR | O_TRUNC
 * 6. Open dir O_RDONLY
 * 7. Open dir O_RDWR, expect EISDIR
 * 8. Open regular file O_DIRECTORY, expect ENOTDIR
 * 9. Open hard link file O_RDONLY
 * 10. Open hard link file O_WRONLY
 * 11. Open hard link file O_RDWR
 * 12. Open sym link file O_RDONLY
 * 13. Open sym link file O_WRONLY
 * 14. Open sym link file O_RDWR
 * 15. Open sym link dir O_RDONLY
 * 16. Open sym link dir O_WRONLY, expect EISDIR
 * 17. Open sym link dir O_RDWR, expect EISDIR
 * 18. Open device special file O_RDONLY
 * 19. Open device special file O_WRONLY
 * 20. Open device special file O_RDWR
 * 21. Open non-existing regular file in existing dir
 * 22. Open link file O_RDONLY | O_CREAT
 * 23. Open symlink file O_RDONLY | O_CREAT
 * 24. Open regular file O_RDONLY | O_CREAT
 * 25. Open symlink dir O_RDONLY | O_CREAT, expect EISDIR
 * 26. Open dir O_RDONLY | O_CREAT, expect EISDIR
 * 27. Open regular file O_RDONLY | O_TRUNC, behaviour is undefined but should
 *     not oops or hang
 * 28. Open regular file(non-empty) O_RDONLY | O_TRUNC, behaviour is undefined
 *     but should not oops or hang
 */

#define _GNU_SOURCE
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

#include "test.h"

char *TCID = "open11";

/* Define test files */
#define T_REG "t_reg"			/* regular file with content */
#define T_REG_EMPTY "t_reg_empty"	/* empty regular file */
#define T_LINK_REG "t_link_reg"		/* hard link to T_REG */
#define T_NEW_REG "t_new_reg"		/* new regular file to be created */
#define T_SYMLINK_REG "t_symlink_reg"	/* symlink to T_REG */
#define T_DIR "t_dir"			/* test dir */
#define T_SYMLINK_DIR "t_symlink_dir"	/* symlink to T_DIR */
#define T_DEV "t_dev"			/* test device special file */

#define T_MSG "this is a test string"

static void setup(void);
static void cleanup(void);

struct test_case {
	char *desc;
	char *path;
	int flags;
	mode_t mode;
	int err;
};
struct test_case tc[] = {
	/*
	 * Test open(2) regular file
	 */
	{	/* open regular file O_RDONLY */
		.desc = "Open regular file O_RDONLY",
		.path = T_REG_EMPTY,
		.flags = O_RDONLY,
		.mode = 0644,
		.err = 0,
	},
	{	/* open regular file O_WRONLY */
		.desc = "Open regular file O_WRONLY",
		.path = T_REG_EMPTY,
		.flags = O_WRONLY,
		.mode = 0644,
		.err = 0,
	},
	{	/* open regular file O_RDWR */
		.desc = "Open regular file O_RDWR",
		.path = T_REG_EMPTY,
		.flags = O_RDWR,
		.mode = 0644,
		.err = 0,
	},
	{	/* open regular file O_RDWR | O_SYNC*/
		.desc = "Open regular file O_RDWR | O_SYNC",
		.path = T_REG_EMPTY,
		.flags = O_RDWR | O_SYNC,
		.mode = 0644,
		.err = 0,
	},
	{	/* open regular file O_RDWR | O_TRUNC */
		.desc = "Open regular file O_RDWR | O_TRUNC",
		.path = T_REG_EMPTY,
		.flags = O_RDWR | O_TRUNC,
		.mode = 0644,
		.err = 0,
	},
	/*
	 * Test open(2) directory
	 */
	{	/* open dir O_RDONLY */
		.desc = "Open dir O_RDONLY",
		.path = T_DIR,
		.flags = O_RDONLY,
		.mode = 0755,
		.err = 0,
	},
	{	/* open dir O_RDWR */
		.desc = "Open dir O_RDWR, expect EISDIR",
		.path = T_DIR,
		.flags = O_RDWR,
		.mode = 0755,
		.err = EISDIR,
	},
	{	/* open regular file O_DIRECTORY */
		.desc = "Open regular file O_DIRECTORY, expect ENOTDIR",
		.path = T_REG_EMPTY,
		.flags = O_RDONLY | O_DIRECTORY,
		.mode = 0644,
		.err = ENOTDIR,
	},
	/*
	 * Test open(2) hard link
	 */
	{	/* open hard link file O_RDONLY */
		.desc = "Open hard link file O_RDONLY",
		.path = T_LINK_REG,
		.flags = O_RDONLY,
		.mode = 0644,
		.err = 0,
	},
	{	/* open hard link file O_WRONLY */
		.desc = "Open hard link file O_WRONLY",
		.path = T_LINK_REG,
		.flags = O_WRONLY,
		.mode = 0644,
		.err = 0,
	},
	{	/* open hard link file O_RDWR */
		.desc = "Open hard link file O_RDWR",
		.path = T_LINK_REG,
		.flags = O_RDWR,
		.mode = 0644,
		.err = 0,
	},
	/*
	 * Test open(2) sym link
	 */
	{	/* open sym link file O_RDONLY */
		.desc = "Open sym link file O_RDONLY",
		.path = T_SYMLINK_REG,
		.flags = O_RDONLY,
		.mode = 0644,
		.err = 0,
	},
	{	/* open sym link file O_WRONLY */
		.desc = "Open sym link file O_WRONLY",
		.path = T_SYMLINK_REG,
		.flags = O_WRONLY,
		.mode = 0644,
		.err = 0,
	},
	{	/* open sym link file O_RDWR */
		.desc = "Open sym link file O_RDWR",
		.path = T_SYMLINK_REG,
		.flags = O_RDWR,
		.mode = 0644,
		.err = 0,
	},
	{	/* open sym link dir O_RDONLY */
		.desc = "Open sym link dir O_RDONLY",
		.path = T_SYMLINK_DIR,
		.flags = O_RDONLY,
		.mode = 0644,
		.err = 0,
	},
	{	/* open sym link dir O_WRONLY */
		.desc = "Open sym link dir O_WRONLY, expect EISDIR",
		.path = T_SYMLINK_DIR,
		.flags = O_WRONLY,
		.mode = 0644,
		.err = EISDIR,
	},
	{	/* open sym link dir O_RDWR */
		.desc = "Open sym link dir O_RDWR, expect EISDIR",
		.path = T_SYMLINK_DIR,
		.flags = O_RDWR,
		.mode = 0644,
		.err = EISDIR,
	},
	/*
	 * Test open(2) device special
	 */
	{	/* open device special file O_RDONLY */
		.desc = "Open device special file O_RDONLY",
		.path = T_DEV,
		.flags = O_RDONLY,
		.mode = 0644,
		.err = 0,
	},
	{	/* open device special file O_WRONLY */
		.desc = "Open device special file O_WRONLY",
		.path = T_DEV,
		.flags = O_WRONLY,
		.mode = 0644,
		.err = 0,
	},
	{	/* open device special file O_RDWR */
		.desc = "Open device special file O_RDWR",
		.path = T_DEV,
		.flags = O_RDWR,
		.mode = 0644,
		.err = 0,
	},
	/*
	 * Test open(2) non-existing file
	 */
	{	/* open non-existing regular file in existing dir */
		.desc = "Open non-existing regular file in existing dir",
		.path = T_DIR"/"T_NEW_REG,
		.flags = O_RDWR | O_CREAT,
		.mode = 0644,
		.err = 0,
	},
	/*
	 * test open(2) with O_CREAT
	 */
	{	/* open hard link file O_RDONLY | O_CREAT */
		.desc = "Open link file O_RDONLY | O_CREAT",
		.path = T_LINK_REG,
		.flags = O_RDONLY | O_CREAT,
		.mode = 0644,
		.err = 0,
	},
	{	/* open sym link file O_RDONLY | O_CREAT */
		.desc = "Open symlink file O_RDONLY | O_CREAT",
		.path = T_SYMLINK_REG,
		.flags = O_RDONLY | O_CREAT,
		.mode = 0644,
		.err = 0,
	},
	{	/* open regular file O_RDONLY | O_CREAT */
		.desc = "Open regular file O_RDONLY | O_CREAT",
		.path = T_REG_EMPTY,
		.flags = O_RDONLY | O_CREAT,
		.mode = 0644,
		.err = 0,
	},
	{	/* open symlink dir O_RDONLY | O_CREAT */
		.desc = "Open symlink dir O_RDONLY | O_CREAT, expect EISDIR",
		.path = T_SYMLINK_DIR,
		.flags = O_RDONLY | O_CREAT,
		.mode = 0644,
		.err = EISDIR,
	},
	{	/* open dir O_RDONLY | O_CREAT */
		.desc = "Open dir O_RDONLY | O_CREAT, expect EISDIR",
		.path = T_DIR,
		.flags = O_RDONLY | O_CREAT,
		.mode = 0644,
		.err = EISDIR,
	},
	/*
	 * Other random open(2) tests
	 */
	{	/* open regular file O_RDONLY | O_TRUNC */
		.desc = "Open regular file O_RDONLY | O_TRUNC, "
			"behaviour is undefined but should not oops or hang",
		.path = T_REG_EMPTY,
		.flags = O_RDONLY | O_TRUNC,
		.mode = 0644,
		.err = -1,
	},
	{	/* open regular(non-empty) file O_RDONLY | O_TRUNC */
		.desc = "Open regular file(non-empty) O_RDONLY | O_TRUNC, "
			"behaviour is undefined but should not oops or hang",
		.path = T_REG,
		.flags = O_RDONLY | O_TRUNC,
		.mode = 0644,
		.err = -1,
	},
};

int TST_TOTAL = sizeof(tc) / sizeof(tc[0]);

int main(int argc, char *argv[])
{
	int lc;
	int i;
	int fd;
	int ret;

	tst_parse_opts(argc, argv, NULL, NULL);

	setup();

	for (lc = 0; TEST_LOOPING(lc); lc++) {
		for (i = 0; i < TST_TOTAL; i++) {
			TEST(open(tc[i].path, tc[i].flags, tc[i].mode));
			fd = TEST_RETURN;

			if (tc[i].err == -1 || TEST_ERRNO == tc[i].err) {
				tst_resm(TPASS, "%s", tc[i].desc);
			} else {
				tst_resm(TFAIL | TTERRNO,
					 "%s - expected errno %d - Got",
					 tc[i].desc, tc[i].err);
			}
			if (fd > 0) {
				ret = close(fd);
				if (ret < 0)
					tst_resm(TWARN, "%s - close failed: %s",
						 tc[i].desc, strerror(errno));
			}
		}
	}

	cleanup();
	tst_exit();
}

static void setup(void)
{
	int fd;
	int ret;

	tst_require_root();

	tst_tmpdir();

	/* Create test files */
	fd = open(T_REG, O_WRONLY | O_CREAT, 0644);
	if (fd == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Create %s failed", T_REG);
	ret = write(fd, T_MSG, sizeof(T_MSG));
	if (ret == -1) {
		close(fd);
		tst_brkm(TBROK | TERRNO, cleanup, "Write %s failed", T_REG);
	}
	close(fd);

	fd = creat(T_REG_EMPTY, 0644);
	if (fd == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Create %s failed",
			 T_REG_EMPTY);
	close(fd);

	ret = link(T_REG, T_LINK_REG);
	if (ret == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Hard link %s -> %s failed",
			 T_REG, T_LINK_REG);

	ret = symlink(T_REG, T_SYMLINK_REG);
	if (ret == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Symlink %s -> %s failed",
			 T_REG, T_SYMLINK_REG);

	ret = mkdir(T_DIR, 0755);
	if (ret == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "mkdir %s failed", T_DIR);

	ret = symlink(T_DIR, T_SYMLINK_DIR);
	if (ret == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Symlink %s -> %s failed",
			 T_DIR, T_SYMLINK_DIR);

	ret = mknod(T_DEV, S_IFCHR, makedev(1, 5));
	if (ret == -1)
		tst_brkm(TBROK | TERRNO, cleanup, "Create char dev %s failed",
			 T_DEV);

	TEST_PAUSE;
}

static void cleanup(void)
{
	tst_rmdir();
}
