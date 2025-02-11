// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2013 Red Hat, Inc.
 * Copyright (c) Linux Test Project, 2013-2022
 */

/*\
 * Basic tests for open(2) and make sure open(2) works and handles error
 * conditions correctly.
 *
 * There are 28 test cases:
 *
 * 1. Open regular file O_RDONLY
 * 2. Open regular file O_WRONLY
 * 3. Open regular file O_RDWR
 * 4. Open regular file O_RDWR | O_SYNC
 * 5. Open regular file O_RDWR | O_TRUNC
 * 6. Open directory O_RDONLY
 * 7. Open directory O_RDWR, expect EISDIR
 * 8. Open regular file O_DIRECTORY, expect ENOTDIR
 * 9. Open hard link file O_RDONLY
 * 10. Open hard link file O_WRONLY
 * 11. Open hard link file O_RDWR
 * 12. Open symlink file O_RDONLY
 * 13. Open symlink file O_WRONLY
 * 14. Open symlink file O_RDWR
 * 15. Open symlink directory O_RDONLY
 * 16. Open symlink directory O_WRONLY, expect EISDIR
 * 17. Open symlink directory O_RDWR, expect EISDIR
 * 18. Open device special file O_RDONLY
 * 19. Open device special file O_WRONLY
 * 20. Open device special file O_RDWR
 * 21. Open non-existing regular file in existing dir
 * 22. Open link file O_RDONLY | O_CREAT
 * 23. Open symlink file O_RDONLY | O_CREAT
 * 24. Open regular file O_RDONLY | O_CREAT
 * 25. Open symlink directory O_RDONLY | O_CREAT, expect EISDIR
 * 26. Open directory O_RDONLY | O_CREAT, expect EISDIR
 * 27. Open regular file O_RDONLY | O_TRUNC, behaviour is undefined but should
 *     not oops or hang
 * 28. Open regular file(non-empty) O_RDONLY | O_TRUNC, behaviour is undefined
 *     but should not oops or hang
 */

#define _GNU_SOURCE
#include <errno.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "tst_test.h"

#define MNTPOINT "mntpoint"
#define T_REG "t_reg"			/* regular file with content */
#define T_REG_EMPTY "t_reg_empty"	/* empty regular file */
#define T_LINK_REG "t_link_reg"		/* hard link to T_REG */
#define T_NEW_REG "t_new_reg"		/* new regular file to be created */
#define T_SYMLINK_REG "t_symlink_reg"	/* symlink to T_REG */
#define T_DIR "t_dir"			/* test directory */
#define T_SYMLINK_DIR "t_symlink_dir"	/* symlink to T_DIR */
#define T_DEV MNTPOINT"/t_dev"		/* test device special file */

#define T_MSG "this is a test string"

static struct test_case {
	char *desc;
	char *path;
	int flags;
	mode_t mode;
	int err;
} tc[] = {
	/* Test open(2) regular file */
	{
		.desc = "open regular file O_RDONLY",
		.path = T_REG_EMPTY,
		.flags = O_RDONLY,
		.mode = 0644,
	},
	{
		.desc = "open regular file O_WRONLY",
		.path = T_REG_EMPTY,
		.flags = O_WRONLY,
		.mode = 0644,
	},
	{
		.desc = "open regular file O_RDWR",
		.path = T_REG_EMPTY,
		.flags = O_RDWR,
		.mode = 0644,
	},
	{
		.desc = "open regular file O_RDWR | O_SYNC",
		.path = T_REG_EMPTY,
		.flags = O_RDWR | O_SYNC,
		.mode = 0644,
	},
	{
		.desc = "open regular file O_RDWR | O_TRUNC",
		.path = T_REG_EMPTY,
		.flags = O_RDWR | O_TRUNC,
		.mode = 0644,
	},

	/* Test open(2) directory */
	{
		.desc = "open directory O_RDONLY",
		.path = T_DIR,
		.flags = O_RDONLY,
		.mode = 0755,
	},
	{
		.desc = "open directory O_RDWR",
		.path = T_DIR,
		.flags = O_RDWR,
		.mode = 0755,
		.err = EISDIR,
	},
	{
		.desc = "open regular file O_DIRECTORY",
		.path = T_REG_EMPTY,
		.flags = O_RDONLY | O_DIRECTORY,
		.mode = 0644,
		.err = ENOTDIR,
	},
	/* Test open(2) hard link */
	{
		.desc = "open hard link file O_RDONLY",
		.path = T_LINK_REG,
		.flags = O_RDONLY,
		.mode = 0644,
	},
	{
		.desc = "open hard link file O_WRONLY",
		.path = T_LINK_REG,
		.flags = O_WRONLY,
		.mode = 0644,
	},
	{
		.desc = "open hard link file O_RDWR",
		.path = T_LINK_REG,
		.flags = O_RDWR,
		.mode = 0644,
	},

	/* Test open(2) symlink */
	{
		.desc = "open symlink file O_RDONLY",
		.path = T_SYMLINK_REG,
		.flags = O_RDONLY,
		.mode = 0644,
	},
	{
		.desc = "open symlink file O_WRONLY",
		.path = T_SYMLINK_REG,
		.flags = O_WRONLY,
		.mode = 0644,
	},
	{
		.desc = "open symlink file O_RDWR",
		.path = T_SYMLINK_REG,
		.flags = O_RDWR,
		.mode = 0644,
	},
	{
		.desc = "open symlink directory O_RDONLY",
		.path = T_SYMLINK_DIR,
		.flags = O_RDONLY,
		.mode = 0644,
	},
	{
		.desc = "open symlink directory O_WRONLY",
		.path = T_SYMLINK_DIR,
		.flags = O_WRONLY,
		.mode = 0644,
		.err = EISDIR,
	},
	{
		.desc = "open symlink directory O_RDWR",
		.path = T_SYMLINK_DIR,
		.flags = O_RDWR,
		.mode = 0644,
		.err = EISDIR,
	},

	/* Test open(2) device special */
	{
		.desc = "open device special file O_RDONLY",
		.path = T_DEV,
		.flags = O_RDONLY,
		.mode = 0644,
	},
	{
		.desc = "open device special file O_WRONLY",
		.path = T_DEV,
		.flags = O_WRONLY,
		.mode = 0644,
	},
	{
		.desc = "open device special file O_RDWR",
		.path = T_DEV,
		.flags = O_RDWR,
		.mode = 0644,
	},

	/* Test open(2) non-existing file */
	{
		.desc = "open non-existing regular file in existing dir",
		.path = T_DIR"/"T_NEW_REG,
		.flags = O_RDWR | O_CREAT,
		.mode = 0644,
	},

	/* test open(2) with O_CREAT */
	{
		.desc = "open link file O_RDONLY | O_CREAT",
		.path = T_LINK_REG,
		.flags = O_RDONLY | O_CREAT,
		.mode = 0644,
	},
	{
		.desc = "open symlink file O_RDONLY | O_CREAT",
		.path = T_SYMLINK_REG,
		.flags = O_RDONLY | O_CREAT,
		.mode = 0644,
	},
	{
		.desc = "open regular file O_RDONLY | O_CREAT",
		.path = T_REG_EMPTY,
		.flags = O_RDONLY | O_CREAT,
		.mode = 0644,
	},
	{
		.desc = "open symlink directory O_RDONLY | O_CREAT",
		.path = T_SYMLINK_DIR,
		.flags = O_RDONLY | O_CREAT,
		.mode = 0644,
		.err = EISDIR,
	},
	{
		.desc = "open directory O_RDONLY | O_CREAT",
		.path = T_DIR,
		.flags = O_RDONLY | O_CREAT,
		.mode = 0644,
		.err = EISDIR,
	},

	/* Other random open(2) tests */
	{
		.desc = "open regular file O_RDONLY | O_TRUNC, "
			"behaviour is undefined but should not oops or hang",
		.path = T_REG_EMPTY,
		.flags = O_RDONLY | O_TRUNC,
		.mode = 0644,
		.err = -1,
	},
	{
		.desc = "open regular file(non-empty) O_RDONLY | O_TRUNC, "
			"behaviour is undefined but should not oops or hang",
		.path = T_REG,
		.flags = O_RDONLY | O_TRUNC,
		.mode = 0644,
		.err = -1,
	},
};

static void verify_open(unsigned int n)
{
	if (tc[n].err > 0) {
		TST_EXP_FAIL2(open(tc[n].path, tc[n].flags, tc[n].mode),
		             tc[n].err, "%s", tc[n].desc);
	} else if (tc[n].err == 0) {
		TST_EXP_FD(open(tc[n].path, tc[n].flags, tc[n].mode),
		           "%s", tc[n].desc);
	} else {
		TEST(open(tc[n].path, tc[n].flags, tc[n].mode));
		tst_res(TPASS, "%s", tc[n].desc);
	}

	if (TST_RET > 0)
		SAFE_CLOSE(TST_RET);
}

static void setup(void)
{
	int fd;
	int ret;

	fd = SAFE_OPEN(T_REG, O_WRONLY | O_CREAT, 0644);
	ret = write(fd, T_MSG, sizeof(T_MSG));
	if (ret == -1) {
		SAFE_CLOSE(fd);
		tst_brk(TBROK | TERRNO, "Write %s failed", T_REG);
	}
	SAFE_CLOSE(fd);

	SAFE_TOUCH(T_REG_EMPTY, 0644, NULL);
	SAFE_LINK(T_REG, T_LINK_REG);
	SAFE_SYMLINK(T_REG, T_SYMLINK_REG);
	SAFE_MKDIR(T_DIR, 0755);
	SAFE_SYMLINK(T_DIR, T_SYMLINK_DIR);
	SAFE_MKNOD(T_DEV, S_IFCHR, makedev(1, 5));
}

static struct tst_test test = {
	.tcnt = ARRAY_SIZE(tc),
	.setup = setup,
	.test = verify_open,
	.needs_devfs = 1,
	.mntpoint = MNTPOINT,
	.needs_root = 1,
};
